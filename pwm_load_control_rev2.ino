//PWM PID Test Code
#include <Arduino.h>

#define pwm_pin 6
#define R13_switch 7

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
float i_pcc_avg;    
float r_load;

//Voltage Divider used to input safe voltage
float voltage_pcc_multiplier=6;
float voltage_gen_multiplier=6;

//Current Divider
float mV_A = 175;

float pwm = 255;
int prev_pwm = 0;

float power_load = 0;
float power_opt = 0;

float power_err = 0;
float power_err_old = 0;

float deltaPWM = 0;
float P = 0;
float D = 0;

float Kp = 19;
float Kd = 10;

float voltage_rated_13 = 5;
int timer;
int cycle;

float pwm_err = 0;
float pwm_err_old = 0;

void setup(){
  Serial.begin(9600);
}

void loop(){

  //start timer
  timer = millis()/1000;

  
  //measure raw analog values
  voltage_pcc = analogRead(voltage_pcc_raw);
  current_pcc = analogRead(current_pcc_raw);

  //int power_raw = voltage_pcc*current_pcc;
  
  //calculate actual voltage and current values
  v_pcc = voltage_pcc*(voltage_pcc_multiplier)*(5.00/1023.00);
  i_pcc = (current_pcc/1023.0)*5000;
  
  i_pcc_mA = 1000*(i_pcc - 2512)/mV_A; 

  //add up sample points
  v_pcc_sample += v_pcc;
  i_pcc_sample += i_pcc_mA;
  
  //Calculate Power into Load
  
  //calculate the average of the data sampled
  if (cycle == 1)
  { 
    v_pcc_avg = v_pcc_sample / 1;
    i_pcc_avg = i_pcc_sample / 1;
    
    r_load = 1000*(v_pcc / i_pcc_mA);
    
    v_gen_sample = 0;
    v_pcc_sample = 0;
    i_pcc_sample = 0;

    power_load  = (v_pcc_avg*i_pcc_avg)/1000;
    //Calculate Power Error
    //power_err =power_opt-(power_load*1000);
    //power_opt = power_load;
    //power_opt = power_load*1000;

    power_err = (1.5 - power_load)*1000;
    pwm_err = abs(power_err);
    
    if (power_load > 1.5 && (v_pcc_avg-0.25> voltage_rated_13 || v_pcc_avg-0.15 < voltage_rated_13))
    {
      digitalWrite(R13_switch, HIGH);  
    }

    else if(power_load < 1 && v_pcc_avg<voltage_rated_13-2)
    {
      digitalWrite(R13_switch, LOW);  
    }
    
    //decide whether PWM needs to be adjusted
    if(power_err < 50 && power_err > -50)
    {
      //delay(10);
    }
    else
    {
      
      P = 17*pwm_err/1000;
      D = 21*(pwm_err - pwm_err_old)/2000;
  
      pwm_err_old = pwm_err;
      
      deltaPWM = P+D;
      
      prev_pwm = pwm_err;
      pwm = constrain(pwm,0,255);
      
      if(power_err < -50)
      {
        //power optimal not reached, adjust PWM to be lower for higher duty cycle which will result in higher current
        pwm = pwm - deltaPWM;
        pwm = constrain(pwm,0,255);
      }
      else if(power_err > 50)
      {
        //power optimal passed, adjust PWM to be higher for lower duty cycle which result in lower current 
        pwm = pwm + deltaPWM;
        pwm = constrain(pwm,0,255);  
      }
      
      //delay(10);
    }
  //update PWM signal
  analogWrite(pwm_pin,pwm);

  cycle = 0;
  }
    //print data
  Serial.print("Time ");
  Serial.print(timer);
  Serial.print(" ");
  Serial.print("Voltage at PCC "); 
  Serial.print(v_pcc_avg);
  Serial.print(" ");
  Serial.print("Current Raw ");
  Serial.print(i_pcc);
  Serial.print(" ");
  Serial.print("Current ");
  Serial.print(i_pcc_avg);
  Serial.print(" ");
  Serial.print("Power Error ");
  Serial.print(power_err);
  Serial.print(" "); 
  Serial.print("Power "); 
  Serial.print(power_load);
  Serial.print(" ");
  Serial.print("Resistance ");
  Serial.print(r_load);
  Serial.print("PWM ");
  Serial.print(pwm);
  
  Serial.print("\n");
  //Monitors the rate of change in Current. If disconnected the following condition will be met and the 
  //turbine will engage into brake mode

  cycle++;
  //cycle_q++;
  
  delay(10); // 50 ms
  
}

