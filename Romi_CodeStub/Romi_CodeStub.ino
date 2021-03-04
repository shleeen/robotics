/*
       @@@@@@@@@@@&*           %@@@@@%       @@@@@@@@@    @@@@@@@@@  @@@@@@@@
       @@@@@@@@@@@@@@@     #@@@@@@@@@@@@    @@@@@@@@@@   @@@@@@@@@* @@@@@@@@@
       @@@@@@   @@@@@@   /@@@@@%  .@@@@@@    @@@/@@@@@ @@@@@@@@@@    @@@@@@
      &@@@@@##&@@@@@@   @@@@@@(   @@@@@@@   @@@,.@@@@@@@@,.@@@@@    @@@@@@
      @@@@@@@@@@@@@    &@@@@@@    @@@@@@   @@@@  @@@@@@@  @@@@@    (@@@@@
     @@@@@@  @@@@@@*   @@@@@@    @@@@@@   .@@@   @@@@@#  @@@@@@    @@@@@&
   @@@@@@@@   @@@@@@%  .@@@@@@@@@@@@@    @@@@@%  @@@@  @@@@@@@@  @@@@@@@@
  %@@@@@@@&   @@@@@@     #@@@@@@@@      @@@@@@   @@@   @@@@@@@/ @@@@@@@@%

  Provided by Paul O'Dowd Oct 2020
*/

#include "lineSensor.h"
#include "encoders.h"
#include "kinematics.h"
#include "motor.h"
#include "pid.h"

#define LINE_LEFT_PIN A2 //Pin for the left line sensor
#define LINE_CENTRE_PIN A3 //Pin for the centre line sensor
#define LINE_RIGHT_PIN A4 //Pin for the right line sensor
#define L_PWM_PIN 10
#define L_DIR_PIN 16
#define R_PWM_PIN  9
#define R_DIR_PIN 15

const int THRESHOLD = 500;

int state = 0;

LineSensor_c sensor_L (LINE_LEFT_PIN); //a line sensor object for the left sensor
LineSensor_c sensor_C (LINE_CENTRE_PIN);
LineSensor_c sensor_R (LINE_RIGHT_PIN);

Motor_c motor_L (L_PWM_PIN, L_DIR_PIN);
Motor_c motor_R (R_PWM_PIN, R_DIR_PIN);

// drives straight
void DriveStraight() {
  motor_L.setMotorPower(20);
  motor_R.setMotorPower(20);
}

//
float WeightedCalc(){
  float I_l = sensor_L.getVoltage();
  float I_c = sensor_C.getVoltage();
  float I_r = sensor_R.getVoltage();

  float I_total = I_l + I_c + I_r;
  float prob[3];
  prob[0] = I_l/I_total;
  prob[1] = I_c/I_total;
  prob[2] = I_r/I_total;

  float M = prob[0] - prob[1];

  return M;
}

// BangBang controller with power scaling
void BangBangPower(){
  float power_r = M * power_max * (-1);
  float power_left = M * power_max * (1);
  
  
}

// bang bang controller for romi to follow line
void BangBang(){
  Serial.print("in bang bang \n");
  
  if (sensor_L.onLine(THRESHOLD) && sensor_C.onLine(THRESHOLD) && sensor_R.onLine(THRESHOLD) ){
    Serial.print("hello \n");
    // move forward
    motor_L.setMotorPower(20);
    motor_R.setMotorPower(20);
  }
  else if(sensor_L.onLine(THRESHOLD) && !sensor_R.onLine(THRESHOLD) ) {
    //if only Left is on line, move LEFT???
    Serial.print("there \n");
    motor_R.setMotorPower(15);
    motor_L.setMotorPower(0);
  }
  else {
    motor_L.setMotorPower(0);
    motor_R.setMotorPower(0);
  }
//  else if(){
//    //if only CENTRE is on line
//    //move STRAIGHT???
//  }
//  else if(){
//    //if only RIGHT is on line
//    //move RIGHT???
//  }

//  if M > 0, move right
// else if M < 0, move left
// else (M == 0), move straight
}

// Setup, only runs once when the power is turned on.
//However, if your Romi gets reset, it will run again.
void setup() {
  // Start up the serial port.
  Serial.begin(9600);

  // Delay to connect properly.
  delay(1000);

  // Calibrate the three line sensors.
  sensor_L.calibrate();
  sensor_C.calibrate();
  sensor_R.calibrate();

  // beep?

  // Print a debug, so we can see a reset on monitor.
  Serial.println("***RESET***");

} // end of setup()



void loop() {
  Serial.println(" In loop ");
  delay(20);

  // call bang bang
  
}
