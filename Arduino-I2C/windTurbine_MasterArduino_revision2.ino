// Wire Master Reader
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Reads data from an I2C/TWI slave device
// Refer to the "Wire Slave Sender" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

int send_data = 0;
int receive_data = 0;
uint8_t incoming_data[4];
int turbine_state = 0;

float t_float;
char t[4];

void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  pinMode(4,OUTPUT);
  
  Serial.begin(9600);  // start serial for output
}



void loop(){


  Serial.println("Load On, Waking up Turbine Now");

  delay(20);

  
  while(receive_data==0){

    //Power flow from Load to Turbine and wait for communication from the turbine
    initiate_master_slave();
  }

  //Master has acknowledged that turbine is ready for operation and will stop power flow from Load to Turbine
  Serial.println("Load is Connected. Byte Received is ");
  Serial.print(receive_data);
  digitalWrite(4,LOW);
  delay(1000);

  //Waiting on Turbine to power up Slave Arduino - Turbine State = 2 
  while(receive_data !=2)
  {
    wait_turbine_powerup();
  }

  Serial.println("Turbine has Started Up, Receiving Data Now");
  
  while(receive_data!=0)
  { 
    request_data();

    //Shutdown if Load has been disconnected or Manual Shutdown has been initiated
    if (turbine_state == 3)
    {
      Serial.println("Wind Turbine Shutting Down");
      delay(5000);
      reset_variables();
      return;
    }
    else
    {
      
      //Save received data into appropriate vatriables
      turbine_state = incoming_data[0];
      int i_pcc_mA = incoming_data[1];
      int v_pcc = incoming_data[2];
      int rps = incoming_data[3];
      Serial.print("data");
      Serial.print("");
      Serial.print(turbine_state);
      Serial.print(" ");
      Serial.print(i_pcc_mA);
      Serial.print(" ");
      Serial.print(v_pcc);
      Serial.print(" ");
      Serial.println(rps);
      /*
      t_float = atof(t);
      t_float = map(t_float, 0.00, 1023.00, 1023.00, 0.00) - 325.00;
      t_float = t_float*5.00/1023.00;
      Serial.print(t_float);
      delay(10);
      */
    }
    
    delay(100);
  }
  
    
  delay(500);
  
  
}

void initiate_master_slave(){

    //Turbine State will be equal to one
    send_data=1;

    //Allow Power to Flow to the Wind Turbine
    digitalWrite(4,HIGH);

    //Poke slave with a message
    delay(10);
    Wire.beginTransmission(9);
    Wire.write(send_data);
    Wire.endTransmission();
    
    Serial.println("Waiting on Turbine");
    delay(100);

    //wait for a response from the turbine
    Wire.beginTransmission(9);
    Wire.requestFrom(9, 8);
    if(Wire.available())
    {
      receive_data=Wire.read();
      Serial.println(receive_data);
    }
    Wire.endTransmission();
}

void wait_turbine_powerup(){
    send_data = 2;
    Wire.beginTransmission(9);
    Wire.write(send_data);
    Wire.endTransmission();

    Serial.print("Waiting on Turbine to begin Producing Power");
    delay(10);

    Wire.beginTransmission(9);
    Wire.requestFrom(9, 1);
    if(Wire.available())
    {
      receive_data=Wire.read();
      Serial.println(receive_data);
    }
    Wire.endTransmission();
}

void request_data(){
    Wire.beginTransmission(9);
    Wire.requestFrom(9,8);
    
    delay(100);
    
    int i = 0;
    while(Wire.available())
    {
      
      incoming_data[i] = Wire.read();
      i++;
    }
    
    Wire.endTransmission();
}

void reset_variables(){
    receive_data = 0;
    turbine_state = 0;
    send_data = 0;
}

