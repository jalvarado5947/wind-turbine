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


void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  pinMode(4,OUTPUT);
  
  Serial.begin(9600);  // start serial for output
}



void loop(){


  Serial.println("Load On, Waking up Turbine Now");

  delay(20);
  
  while(receive_data==0){

    send_data=1;

    //Allow Power to Flow to the Wind Turbine
    digitalWrite(4,HIGH);
    
    delay(10);
    Wire.beginTransmission(9);
    Wire.write(send_data);
    Wire.endTransmission();
    
    Serial.println("Waiting on Turbine");
    delay(100);

    Wire.beginTransmission(9);
    Wire.requestFrom(9, 4);
    if(Wire.available())
    {
      receive_data=Wire.read();
      Serial.println(receive_data);
    }
    Wire.endTransmission();
    /*
    if(receive_data == 1)
    {
      Serial.println("Make sure Load is Connected");
    }
    */
    //Serial.println(receive_data);
  }
  
  Serial.println("Load is Connected. Byte Received is ");
  Serial.print(receive_data);
  digitalWrite(4,LOW);
  delay(1000);

  while(receive_data !=2)
  {
    send_data = 2;
    Wire.beginTransmission(9);
    Wire.write(send_data);
    Wire.endTransmission();

    Serial.print("Waiting on Turbine to begin Producing Power");
    delay(100);

    Wire.beginTransmission(9);
    Wire.requestFrom(9, 1);
    if(Wire.available())
    {
      receive_data=Wire.read();
      Serial.println(receive_data);
    }
    Wire.endTransmission();
    

  }

  Serial.println("Turbine has Started Up, Receiving Data Now");
  
  while(receive_data!=0)
  {

    //byte error;
    Wire.beginTransmission(9);
    Wire.requestFrom(9,4);
    
    
    delay(100);

    /*
    if(Wire.available()==0)
    {
      receive_data = 0;
      Serial.println("Turbine was Disconnected. Restarting Now");
      delay(2000);
      return;
    }
    else{
      receive_data = Wire.read();
      Serial.print
    }
    */
    int i = 0;
    while(Wire.available())
    {
      
      incoming_data[i] = Wire.read();
      i++;
    }
    
    Wire.endTransmission();
    
    int turbine_state = incoming_data[0];
    if (turbine_state == 3)
    {
      Serial.println("Wind Turbine Shutting Down");
      receive_data = 0;
      delay(5000);
      return;
    }
    else
    {
      int turbine_state = incoming_data[0];
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
    }
    
    delay(100);
  }
  
    
  delay(500);
  
  
}

/*
void loop() {
  if (x == 10)
  {
    Wire.requestFrom(8, 6);    // request 6 bytes from slave device #8
    x=0;
  }
  while (Wire.available()) { // slave may send less than requested
    char c = Wire.read(); // receive a byte as character
    Serial.print(c);         // print the character
  }
  x++;

  delay(500);
}

*/
