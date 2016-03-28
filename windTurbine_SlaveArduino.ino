#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#define CS                    3
#define brake_relay_engage    4
#define brake_relay_disengage 5
#define relay_1               6

byte y = 0;


float voltage_pcc_raw = A0;
float current_pcc_raw = A1;

float voltage_pcc;
float current_pcc;

float v_pcc_sample = 0;
float i_pcc_sample = 0;
float i_pcc_mA_sample = 0;

float v_pcc;
float i_pcc;
float i_pcc_mA;

float power_load;
float power_rated = 4.25;
float r_load = 0;

//Voltage Divider used to input safe voltage: R1 =   ; R2 =   ;
float voltage_pcc_multiplier=6;
float mV_A = 175;


int cycle = 0;

float i_pcc_previous= 0;
float i_pcc_error =0;

int cycle_q = 0; 
float i_pcc_sample_q = 0;
float i_pcc_mA_q = 0;
float i_pcc_q = 0;


int region = 1;
//internal clock
int timer = 0;

uint16_t ABSposition = 0;
uint16_t ABSposition_last = 0;
uint8_t temp[1];

float deg = 0.00;

int pin = 2;
int ticks = 0;
int time = 0;
int oneSecond = 0;
int rps = 0;

int receive_data = 0;
char send_data = 0;
int turbine_state = 0;
uint8_t buff[4];
int cycles;
float power;

void setup() {
  
  Wire.begin(9);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent);
  
  pinMode(6,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(4,OUTPUT);

  
  attachInterrupt(0, tick, FALLING);
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV32);
  
  Serial.begin(9600);
  delay(1000);
  
}

void loop() {

  disengage_brake();
  while(turbine_state == 2)
  {
    //This Relay allows current to flow into the load
    digitalWrite(6,HIGH);
    
    //Voltage at PCC divided by the voltage divider
    //Voltage at PCC divided by the voltage divider
    voltage_pcc = analogRead(voltage_pcc_raw);
    current_pcc = analogRead(current_pcc_raw);
    
    timer = millis()/1000; 
    
    //Actual Voltage and Current Value at PCC 
    v_pcc_sample += voltage_pcc*(voltage_pcc_multiplier)*(5.00/1023.00);
    i_pcc_sample += (current_pcc/1023.0)*5000;
    
    i_pcc_sample_q += current_pcc*(5000.0/1023.0);
    
    if(cycle_q == 5)
    {
      i_pcc_q = i_pcc_sample_q / 5;
      
      i_pcc_mA_q = 1000*(i_pcc_q-2514)/mV_A;
      
      i_pcc_sample_q = 0;
      
      cycle_q = 0;
    }
    
    i_pcc_error = (i_pcc_mA_q - i_pcc_mA)/10;
    
    if(i_pcc_mA < 20 && i_pcc_error < -15 && timer > 2)
    {
      turbine_state = 3;
      delay(1000);
    }
    
    if (cycle == 10)
    { 
      v_pcc = v_pcc_sample / 10;
      i_pcc = i_pcc_sample / 10;
      
      i_pcc_mA = 1000*(i_pcc-2514)/mV_A;
      
      power_load = (v_pcc*i_pcc_mA)/1000; 
      
      r_load = 1000*(v_pcc / i_pcc_mA);
      
      v_pcc_sample = 0;
      i_pcc_sample = 0;
     
      cycle = 0;
    }
    
    //print data
    Serial.print("Time ");
    Serial.print(timer);
    Serial.print(" ");
    Serial.print("Voltage "); 
    Serial.print(v_pcc);
    Serial.print(" ");
    Serial.print("Current ");
    Serial.print(i_pcc_mA_q);
    Serial.print(" ");
    Serial.print("Current Error ");
    Serial.print(i_pcc_error);
    Serial.print(" "); 
    Serial.print("Power "); 
    Serial.print(power_load);
    Serial.print(" ");
    Serial.print("Resistance ");
    Serial.print(r_load);
    
    Serial.print("\n");
    //Monitors the rate of change in Current. If disconnected the following condition will be met and the 
    //turbine will engage into brake mode

    cycle++;
    cycle_q++;
    
    delay(10); // 50 ms
  }
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()

void receiveEvent(int value) {

  while(Wire.available()){
    receive_data = Wire.read();    // receive byte as an integer
  }
  if(receive_data == 1)
  {
    turbine_state = 1;
  }
  if(receive_data == 2)
  {
    turbine_state = 2;
  }
}
//********************************************I2C Events***************************************************

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
      Wire.write(buff,4);
    }
    
    
}

//*************************************************Functions*********************************************

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

void tick() {
    ticks++;

    if(ticks == 1000)
    {
      cycle++;
      ticks = 0;
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



void disengage_brake(){
  digitalWrite(5,HIGH);
  digitalWrite(4,LOW);
}
void apply_brake(){
  digitalWrite(4,HIGH);
  digitalWrite(5,LOW);
  digitalWrite(6, LOW);
}
