//PWM PID Test Code
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>


#define CS            3
#define pwm_pin       6
#define R13_switch    8
#define brake_relay   5
#define start_relay   4


uint16_t ABSposition = 0;
uint16_t ABSposition_last = 0;
uint8_t temp[1];

float voltage_gen_raw = A0;
float current_pcc_raw = A1;
float voltage_pcc_raw = A2;

float voltage_gen;
float voltage_pcc;
float current_pcc;

float v_gen_sample = 0;
float v_pcc_sample = 0;
float i_pcc_sample = 0;
float i_pcc_mA_sample = 0;

float v_gen;
float v_pcc;
float i_pcc;
float i_pcc_mA;

//average of sample data
float v_pcc_avg;
float v_gen_avg;
float i_pcc_avg;    
float r_load;

//Voltage Divider used to input safe voltage
float voltage_pcc_multiplier=5.8;
float voltage_gen_multiplier=5.8;

//Current Divider
float mV_A = 185;

float pwm = 10;
int prev_pwm = 0;

float power_load = .2;
float power_opt = 0;
float power_rated = 5;


float power_err = 0;
float power_err_old = 0;

float power_max = 0;

float deltaPWM = 0;
float P = 0;
float D = 0;

float Kp = 19;
float Kd = 10;

float voltage_rated_13 = 6;
int timer;
int cycle;
int cycle_q;

float i_pcc_sample_q = 0;
float i_pcc_mA_q = 0;
float i_pcc_q = 0;

float current_pcc_q;
float i_pcc_avg_q;


float i_pcc_error = 0;

float pwm_err = 0;
float pwm_err_old = 0;

int pin = 2;
int ticks = 0;
int time = 0;
int oneSecond = 0;
int rps = 0;
int rev = 0;
int rev_x;
float deg = 0.00;
int timer2 =0;
int pwm_opt;

int receive_data = 0;
char send_data = 0;
int turbine_state = 0;
uint8_t buff[4];
//int cycles;
//float power;

char i[4];
int x = 0;

void setup(){
  Wire.begin(9);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent);
  
  Serial.begin(115200);
  pinMode(R13_switch,OUTPUT);
  pinMode(brake_relay,OUTPUT);
  pinMode(start_relay,OUTPUT); 
   

  digitalWrite(brake_relay, HIGH);
  digitalWrite(start_relay, LOW);

  // put your setup code here, to run once:
  attachInterrupt(0, tick, FALLING);
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV32);
  
  delay(1000);
}



void loop(){
  //this while statement is used during communication between both arduinos, not needed for testing the power 
  while(turbine_state == 0){
    
    disengage_brake();
    //delay(2000);
    //brake_turbine();
    //start timer
    timer = millis()/1000;
    int time_diff = millis()- timer2;
    if (time_diff >= 950)
    {
      rev = rev_x;
      rev_x=0;
      timer2 = millis(); 
    }
    
    for(cycle = 0; cycle < 5; cycle++){
      //measure raw analog values
      current_pcc_q = analogRead(current_pcc_raw);
      
      //calculate actual voltage and current values
      i_pcc_q = (current_pcc_q/1023.0)*5000;
      
      i_pcc_mA_q = 1000*(i_pcc_q - 2506)/mV_A; 
    
      //add up sample points
      i_pcc_sample_q +=i_pcc_mA_q;
    }
  
    i_pcc_avg_q = i_pcc_sample_q / 5; 
  
  
    i_pcc_error = (i_pcc_mA_q - i_pcc_avg);
    /*
    if(i_pcc_avg_q < 100 &&i_pcc_error < -50 && timer > 4)
    {
      while(1){ 
        brake_turbine();
      }
    }
    */
  
    //Sample the Raw sensor values 10 times
    sample_sensor_values();
  
    //avg the values of the sensors
    v_gen_avg = v_gen_sample / 10;
    v_pcc_avg = v_pcc_sample / 10;
    i_pcc_avg = i_pcc_sample / 10; 
  
    r_load = (v_pcc_avg/i_pcc_avg)*1000;
    
    v_gen_sample = 0;
    v_pcc_sample = 0;
    i_pcc_sample = 0;
    i_pcc_sample_q = 0;
    
    if (power_load > power_rated + 1){
      digitalWrite(R13_switch, HIGH);
    }
    else{
      digitalWrite(R13_switch, LOW);
    }  
    
    delay(200);
    //calculate the power of the load
    power_load = v_pcc_avg * i_pcc_avg / 1000;
    
    power_err = (power_load - power_opt);    // HENRY
    
    if(power_load >= power_opt){
        power_opt = power_load-.5;
    }
    
    pwm_err_old = pwm_err;
    pwm_err = abs(power_err);
    
  
    //Regulate the power past Rated Power at 11 m/s
    if (power_load > 12 && (v_pcc_avg-0.25> voltage_rated_13 || v_pcc_avg-0.25 < voltage_rated_13))
    {
      //digitalWrite(R13_switch, HIGH);
      pwm = pwm - 2;
      analogWrite(pwm_pin, pwm ); 
    }
  
    else if(power_load < 10 && v_pcc_avg<voltage_rated_13-2)
    {
      digitalWrite(R13_switch, LOW);  
    }
    
    //Adjust PWM based on Error of the Power
    adjust_pwm();
    analogWrite(pwm_pin, pwm );
  
        
  //////////////////////////////////////////////// PRINT STUFF //////////////////////////////////////////////////
    while(Serial.available())
    {
      i[x] = Serial.read();
      x++;
      delay(10);
    }
    delay(10);
    
    while(i[0] == '2')
    {
      brake_turbine();
      i[x] = Serial.read();
      delay(10);
      
    }
    
    //This line of code is used with the python code. 
    //i[0] = '1'; If you are not using the python code un comment this line so that you can print out to the command prompt on arduino
    if(i[0] == '1')
    {
      int rpm = 60*rev;
      //print data
      Serial.print(timer);
      Serial.print("\t");
      //Serial.print(i[1]);
      //Serial.print(i[2]);    
      //Serial.print("\t");
      Serial.print(v_gen_avg);
      Serial.print("\t");
      //Serial.print(v_pcc_avg);
      //Serial.print("\t");
      Serial.print(i_pcc_mA);
      Serial.print("\t");
      Serial.print(i_pcc_error);
      Serial.print("\t");
      //Serial.print(power_err);
      //Serial.print(" \t "); 
      Serial.print(power_load);
      Serial.print("\t");
      Serial.print(power_err);
      Serial.print("\t");
      Serial.print(r_load);
      Serial.print("\t ");
      //Serial.print(pwm);
      //Serial.print("\t ");
      Serial.print(rpm);
      Serial.print("\t ");
      Serial.print(pwm);
      Serial.print("\n");
      //Monitors the rate of change in Current. If disconnected the following condition will be met and the 
      //turbine will engage into brake mode
    }
    else if(i[0] == '0')
    { 
       delay(10);
    }
    x = 0;
  
  
    cycle++;
    cycle_q++;
  
    delay(200); // 50 ms
  }
}

void brake_turbine(){
  digitalWrite(brake_relay, HIGH);
  digitalWrite(start_relay, LOW);
}

void disengage_brake(){
  digitalWrite(brake_relay,LOW);
  digitalWrite(start_relay,HIGH);
}

void requestEvent() {
    
    buff[0] = turbine_state;
    buff[1] = i_pcc_mA;
    buff[2] = v_pcc;
    buff[3] = rps;
  
    if(receive_data == 1)
    {
      disengage_brake();
      Wire.write(buff,4);
    }
    if(receive_data == 2)
    {
      //turbine_turbine = 2;
      Wire.write(buff,1);
    }
    
    
}

void receiveEvent(int value) {

  while(Wire.available()){
    receive_data = Wire.read();    // receive byte as an integer
  }
  if(receive_data == 1)
  {
    turbine_state = 1;
  }
  if(receive_data == 2 && turbine_state == 1)
  {
    turbine_state = 2;
  }
}

void shaft_rps()
{
   uint8_t recieved = 0xA5;    //just a temp vairable
   ABSposition = 0;    //reset position vairable
   
   time = millis()/1000;
   
   SPI.begin();    //start transmition
   digitalWrite(CS,LOW);
   
   SPI_T(0x10);   //issue read command
   
   recieved = SPI_T(0x00);    //issue NOP to check if encoder is ready to send
   
   while (recieved != 0x10)    //loop while encoder is not ready to send
   {
     recieved = SPI_T(0x00);    //cleck again if encoder is still working 
     delay(2);    //wait a bit
   }
   
   temp[0] = SPI_T(0x00);    //Recieve MSB
   temp[1] = SPI_T(0x00);    // recieve LSB
   
   digitalWrite(CS,HIGH);  //just to make sure   
   SPI.end();    //end transmition
   
   temp[0] &=~ 0xF0;    //mask out the first 4 bits
    
   ABSposition = temp[0] << 8;    //shift MSB to correct ABSposition in ABSposition message
   ABSposition += temp[1];    // add LSB to ABSposition message to complete message
    
   if (ABSposition != ABSposition_last)    //if nothing has changed dont wast time sending position
   {
     ABSposition_last = ABSposition;    //set last position to current position
     deg = ABSposition;
     deg = deg * 0.08789;    // aprox 360/4096
     
   }   
}


uint8_t SPI_T (uint8_t msg)    //Repetive SPI transmit sequence
{
   uint8_t msg_temp = 0;  //vairable to hold recieved data
   digitalWrite(CS,LOW);     //select spi device
   msg_temp = SPI.transfer(msg);    //send and recieve
   digitalWrite(CS,HIGH);    //deselect spi device
   return(msg_temp);      //return recieved byte
}

void tick() {
    ticks++;

    if(ticks == 1000)
    {
      rev_x++;
      ticks = 0;
    }
   
}

void sample_sensor_values(){
  for(cycle = 0; cycle < 10; cycle++){
    //measure raw analog values
    voltage_pcc = analogRead(voltage_pcc_raw);
    current_pcc = analogRead(current_pcc_raw);
    voltage_gen = analogRead(voltage_gen_raw);
    
    //calculate actual voltage and current values
    v_gen = voltage_gen*(voltage_gen_multiplier)*(5.00/1023.00);
    v_pcc = voltage_pcc*(voltage_pcc_multiplier)*(5.00/1023.00);
    i_pcc = (current_pcc/1023.0)*5000;
    
    i_pcc_mA = 1000*(i_pcc - 2508)/mV_A; 
  
    //add up sample points
    v_gen_sample += v_gen;
    v_pcc_sample += v_pcc;
    i_pcc_sample += i_pcc_mA;
    i_pcc_sample_q +=i_pcc_mA;
  
  }
}

void adjust_pwm(){
  if(power_err < .1 && power_err > -.1 )
  {
    delay(10);
  }
  else
  {
    float Pgain = 2;
    float Dgain =.05;
    float D_timer = 0;
    
    //D = Dgain*(pwm_err - pwm_err_old);    

    D_timer = timer;
    
    P = Pgain*pwm_err;      
    
    if(power_err > .5 ){
      deltaPWM = P+D;
      pwm =pwm+deltaPWM ;
      prev_pwm = pwm;
    }
    else if (power_err < .45){
      deltaPWM = P+D;
      pwm = pwm-deltaPWM;
      
    }
       
    pwm = constrain(pwm,0,255);

    delay(10);
  }
}
