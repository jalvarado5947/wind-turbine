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

float mV_A = 188;
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
 
  
  
  timer = millis()/1000; 
  
  //Actual Voltage and Current Value at PCC 
  v_pcc_sample += voltage_pcc*(voltage_pcc_multiplier)*(5.00/1023.00);
  i_pcc_sample += (current_pcc/1023.0)*5000;
  
  i_pcc_sample_q += current_pcc*(5000.0/1023.0);
  
  v_gen_sample += voltage_gen*(voltage_gen_multiplier)*(5.00/1023.00);
  
  if(cycle_q == 5)
  {
    i_pcc_q = i_pcc_sample_q / 5;
    
    i_pcc_mA_q = 1000*(i_pcc_q-2500)/mV_A;
    
    i_pcc_sample_q = 0;
    
    cycle_q = 0;
  }
  
  i_pcc_error = (i_pcc_mA_q - i_pcc_mA)/100;
  
  if (cycle == 10)
  { 
    v_pcc = v_pcc_sample / 10;
    v_gen = v_gen_sample / 10;
    i_pcc = i_pcc_sample / 10;
    
    i_pcc_mA = 1000*(i_pcc-2513)/mV_A;
    
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
  Serial.print("Time ");
  Serial.print(timer);
  Serial.print(" ");
  Serial.print("Voltage "); 
  Serial.print(v_pcc);
  Serial.print(" ");
  Serial.print("raw current ");
  Serial.print(i_pcc);
  Serial.print(" ");
  Serial.print("Current ");
  Serial.print(i_pcc_mA);
  Serial.print(" "); 
  Serial.print("Power "); 
  Serial.print(power_load);
  Serial.print(" ");
  Serial.print("Resistance ");
  Serial.print(r_load);

  Serial.print("\n");
  
  cycle++;
  cycle_q++;
  
  delay(10); // 50 ms
}


