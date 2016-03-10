#include "Arduino.h"
//setup variables


#define brake_relay_1 4

#define relay_1       5
#define relay_3       6
#define relay_4       7
  


float voltage_pcc_raw = A0;
float current_pcc_raw = A1;
float voltage_gen_raw = A2;


float voltage_pcc;
float voltage_gen;
float current_pcc;

float v_pcc_sample = 0;
float v_gen_sample = 0;
float i_pcc_sample = 0;
float i_pcc_mA_sample = 0;

float v_pcc;
float v_gen;
float i_pcc;
float i_pcc_mA;

float power_load;
float power_rated = 4.25;
float r_load = 0;

//Voltage Divider used to input safe voltage: R1 =   ; R2 =   ;
float voltage_pcc_multiplier=6.8;
float voltage_gen_multiplier=6.8;


int region = 1;
//internal clock
int timer = 0;

int voltage_14ms = 6;

int cycle = 0;
int brake = 1;
int pwm = 0;

/////////////////////////////Practice Variables///////////////////////////////////////////////////////////////
float i_pcc_previous= 0;
float i_pcc_error =0;

int cycle_q = 0; 
float i_pcc_sample_q = 0;
float i_pcc_mA_q = 0;
float i_pcc_q = 0;

float power_previous;
float power_error;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  // put your setup code here, to run once:
  pinMode(relay_1,OUTPUT);
  pinMode(relay_3,OUTPUT);
  pinMode(relay_4,OUTPUT);
  
  pinMode(brake_relay_1,OUTPUT);    
  
  
  Serial.begin(9600);
  delay(1000);
  
  
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  // put your main code here, to run repeatedly:
 
  //Voltage at PCC divided by the voltage divider
  voltage_pcc = analogRead(voltage_pcc_raw);
  voltage_gen = analogRead(voltage_gen_raw);
  current_pcc = analogRead(current_pcc_raw);
 
  
  //Set operating Region
     
  //Jump to shutdown mode
  if( v_pcc > 9 && i_pcc_error < -0.4 && i_pcc_mA_q < 35)
  {
    region = 5;
  }
  
  //engage braking mode
  if(v_pcc > 9 && region == 3)
  {
    region = 4;
  }
  
  if(v_pcc > 5 && region == 2)
  {
    //Turn on dump load for speed and power control 
    region = 3;
  }
  
  if(v_pcc > .9 && region == 1)
  {
    //Allow power flow to Load
    region = 2;
  }
  
  
  if (region == 2)
  {
    digitalWrite(relay_1, HIGH);
    digitalWrite(relay_3, LOW);
    digitalWrite(relay_4, LOW);

    digitalWrite(brake_relay_1, LOW);
  }
  else if(region == 3)
  {
    digitalWrite(relay_1, HIGH);
    digitalWrite(relay_3, HIGH);
    digitalWrite(relay_4, LOW);

    digitalWrite(brake_relay_1, LOW);
  }
  else if(region == 4)
  {
    digitalWrite(relay_1, HIGH);
    digitalWrite(relay_3, HIGH);
    digitalWrite(relay_4, HIGH);

    digitalWrite(brake_relay_1, LOW);
  }
  else if(region == 5)
  {
    digitalWrite(relay_1, LOW);
    digitalWrite(relay_3, LOW);
    digitalWrite(relay_4, LOW);

    digitalWrite(brake_relay_1, HIGH);
  }
  /*
  switch (region){
    //Region 1 operation: very high resistance for minimal load
    //Load switch should be set to highest resistance
    //System switch set to allow flow to load
    case 1:
    {
      digitalWrite(relay_1, LOW);
      digitalWrite(relay_3, LOW);
      digitalWrite(relay_4, LOW);

      digitalWrite(brake_relay_1, LOW);
            
    }
    
    //Region 2 operation: optimal power output. Must match power output by varying resistive load.
    //Actual compared to theoretical 
    case 2:
    {
      digitalWrite(relay_1, HIGH);
      digitalWrite(relay_3, LOW);
      digitalWrite(relay_4, LOW);

      digitalWrite(brake_relay_1, LOW);
      
    }
    //Region 3 operation: Wind speed above 12 m/s. Must regulate power and speed. 
    //Turn on parallel Resistance 
    case 3:
    {
      digitalWrite(relay_1, HIGH);
      digitalWrite(relay_3, HIGH);
      digitalWrite(relay_4, LOW);

      digitalWrite(brake_relay_1, LOW);
    }
    //Region 4 operation: Wind speed above 14m/s. Must initiate braking. 
    case 4:
    {
      digitalWrite(relay_1, HIGH);
      digitalWrite(relay_3, HIGH);
      digitalWrite(relay_4, HIGH);

      digitalWrite(brake_relay_1, LOW);
    }
    
    case 5:
    {
      digitalWrite(relay_1, LOW);
      digitalWrite(relay_3, LOW);
      digitalWrite(relay_4, LOW);

      digitalWrite(brake_relay_1, HIGH);
    }
  }
  */
   
  
  timer = millis()/1000; 
  
  
  
  
  //Actual Voltage and Current Value at PCC 
  v_pcc_sample += voltage_pcc*(voltage_pcc_multiplier)*(5.00/1023.00);
  i_pcc_sample += current_pcc*(5000.0/1023.0);
  
  i_pcc_sample_q += current_pcc*(5000.0/1023.0);
  
  v_gen_sample += voltage_gen*(voltage_gen_multiplier)*(5.00/1023.00);
  
  if(cycle_q == 5)
  {
    i_pcc_q = i_pcc_sample_q / 5;
    
    i_pcc_mA_q = 1000*(i_pcc_q-2503)/250.0;
    
    i_pcc_sample_q = 0;
    
    cycle_q = 0;
  }
  
  i_pcc_error = (i_pcc_mA_q - i_pcc_mA)/100;
  
  if (cycle == 20)
  { 
    v_pcc = v_pcc_sample / 20;
    v_gen = v_gen_sample / 20;
    i_pcc = i_pcc_sample / 20;
    
    i_pcc_mA = 1000*(i_pcc-2503)/265.0;
    
    power_load = (v_pcc*i_pcc_mA)/1000; 
    
    r_load = 1000*(v_pcc / i_pcc_mA);
    
    v_pcc_sample = 0;
    i_pcc_sample = 0;
    v_gen_sample = 0;
    
    
  
    cycle = 0;
  }
  
  if ( power_load > power_previous)
  {
    power_error = power_load - power_previous; 
    power_previous = power_load;
    
  }
  else
  {
    power_error = power_load - power_previous; 
  }
    
  
  //print data
  Serial.print(timer);
  Serial.print(" "); 
  Serial.print(v_pcc);
  Serial.print(" ");
  Serial.print(i_pcc_mA);
  Serial.print(" ");  
  Serial.print(power_load);
  Serial.print(" ");
  Serial.print(r_load);
  Serial.print(" ");
  Serial.print(i_pcc_error);
  Serial.print(" ");
  Serial.print(power_error);
  Serial.print(" ");  
  Serial.print(region);
  Serial.print("\n");
  
  cycle++;
  cycle_q++;
  
  delay(10); // 50 ms
}


