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

// define states 
#define STATE_INITIAL         0
#define STATE_DRIVE_FORWARDS  1
#define STATE_FOUND_LINE      2
#define STATE_FOLLOW_LINE     3
#define STATE_LINE_END        4

// define pins
#define LINE_LEFT_PIN A2 //Pin for the left line sensor
#define LINE_CENTRE_PIN A3 //Pin for the centre line sensor
#define LINE_RIGHT_PIN A4 //Pin for the right line sensor
#define L_PWM_PIN 10
#define L_DIR_PIN 16
#define R_PWM_PIN  9
#define R_DIR_PIN 15

#define POWER_MAX 1 // define max power for power scaling

// Global Variables
int state = 0;
const int THRESHOLD = 500;

long last_e0_count; //long since the value can be negative
long last_e1_count;
unsigned timestamp; // to seq out speed update

// variables for hacky right/left turn logic
int flag = 0;
int counter = 0;

// Creating objects of classes
LineSensor_c sensor_L (LINE_LEFT_PIN); //a line sensor object for the left sensor
LineSensor_c sensor_C (LINE_CENTRE_PIN);
LineSensor_c sensor_R (LINE_RIGHT_PIN);

Motor_c motor_L (L_PWM_PIN, L_DIR_PIN);
Motor_c motor_R (R_PWM_PIN, R_DIR_PIN);

kinematics_c kine;


// Setup, only runs once when the power is turned on.
//However, if your Romi gets reset, it will run again.
void setup() {
  // These two function set up the pin change interrupts for the encoders.
  setupEncoder0();
  last_e0_count = count_e0; //capture initial state of e0
  setupEncoder1();
  last_e1_count = count_e1;
  
  // Start up the serial port.
  Serial.begin(9600);

  // Delay to connect properly.
  delay(1000);

  //set up pins for Buzzer and LED
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(6, OUTPUT); //Buzzer pin

  // Set initial state, before robot begins to operate.
  state = STATE_INITIAL;

  // Record an initial timestamp.
  timestamp = millis();
  
  // Print a debug, so we can see a reset on monitor.
  Serial.println("***RESET***");
}


void loop() {
  float M = WeightedCalc();
//  Serial.println(M);

  // Call kinematics update function
  kine.update();


// Based on the value of STATE variable, run code for the appropriate robot behaviour.
    if( state == STATE_INITIAL ) {
        Serial.println( "in initial state" );
        InitialisingBeeps();
    } 
    else if ( state == STATE_DRIVE_FORWARDS ) {
        Serial.println( "in drive forwards state" );
        MoveStraight();
    }
    else if ( state == STATE_FOUND_LINE ) {
        Serial.println( " in found line state " );
        TurnCheckForLine();
    }
    else if ( state == STATE_FOLLOW_LINE ) {
        Serial.println( " in follow line state" );
        BangBang(M);
    }
    else if ( state == STATE_LINE_END ) {
        Serial.println( " in line end state " );
        TurnCheckForLine();
    }
    else {
        Serial.print("System Error, Unknown state: ");
        Serial.println( state );
    }
  
  delay(20);
}


// Function to check clockwise and anti-clockwise for line
void TurnCheckForLine(){
  // when flag is 0, check clockwise
  if (flag == 0) {
    TurnToTarget(90);

    // increment global counter
    counter += 1;

    if ( sensor_C.onLine(THRESHOLD) ){
      state = STATE_FOLLOW_LINE;
      flag = 0;
    }
    else{
      state = STATE_LINE_END;

      // try left turn only after 500 counts/attempts at turning right
      if (counter > 500){ 
        flag = 1;
      }
    }
  }
  // when flag is 1, check anticlockise
  else if (flag == 1){
    TurnToTarget(-90);
    if ( sensor_C.onLine(THRESHOLD) ){
      state = STATE_FOLLOW_LINE;
    }
    else{
      state = STATE_LINE_END;
    }
  }
}


// Function where Romi beeps 5 times while calibrating.
void InitialisingBeeps() {
  PlayBeep(3, 500);

  // Calibrate the three line sensors.
  sensor_L.calibrate();   PlayBeep(3, 500);
  sensor_C.calibrate();   PlayBeep(3, 500);
  sensor_R.calibrate();   PlayBeep(3, 500);

  PlayBeep(3, 500);
  
  // Trigger State Transition
  state = STATE_DRIVE_FORWARDS;
}


// Function that deactivates all motors, and beep once for one second, flash LED for one sec, then do nothing.
void foundLineBeep() {
  motor_L.setMotorPower(0);
  motor_R.setMotorPower(0);
  PlayBeep(3, 1000);
  FlashLED(1000);
}


// Function to drive forward, and change state if line is found 
void MoveStraight() {
  if( sensor_L.onLine(THRESHOLD) || sensor_C.onLine(THRESHOLD) || sensor_R.onLine(THRESHOLD) ) {
    Serial.println("FOUND line! \n");
    foundLineBeep();
    
    // Trigger State Transition
//    state = STATE_FOUND_LINE;
    state = STATE_FOLLOW_LINE;
  }
  else { 
      Serial.println("DRIVING STRAIGHT! \n");
      //keep going straight
      motor_L.setMotorPower(20);
      motor_R.setMotorPower(20);
  }
}

// Function for Weighted Line Sensing; returns M value
float WeightedCalc(){
  float I_l = sensor_L.getVoltage();
  float I_c = sensor_C.getVoltage();
  float I_r = sensor_R.getVoltage();
//  float I_l = sensor_L.readCalibrated();
//  float I_c = sensor_C.readCalibrated();
//  float I_r = sensor_R.readCalibrated();

  float I_total = I_l + I_c + I_r;
  float prob[3];
  prob[0] = I_l/I_total;
  prob[1] = I_c/I_total;
  prob[2] = I_r/I_total;

  float M = prob[0] - prob[1];

  return M;
}


// Function implementing a bang bang controller for romi to follow lines
void BangBang(float M){
  // if any sensor is on the line, then do bangbang
  if (sensor_L.onLine(THRESHOLD) || sensor_C.onLine(THRESHOLD) || sensor_R.onLine(THRESHOLD)) {
    Serial.println("the M thing");
    if (M < 0.0){ //move right
      motor_R.setMotorPower(5);
      motor_L.setMotorPower(18);
    }
    else if (M > 0.0){ //move left
      motor_R.setMotorPower(15);
      motor_L.setMotorPower(5);
    }
    else if (M == 0.0){ //move forwards
      motor_R.setMotorPower(15);
      motor_L.setMotorPower(18);
    }
    else {
      Serial.println(" IN THE BANG BANG ELSE ");
    }
  }
  else{
    // if out of the line, stop moving
    motor_R.setMotorPower(0);
    motor_L.setMotorPower(0);
    // change state
    state = STATE_LINE_END;
  }
}


// Function to turn toward given angle (in degrees)
void TurnToTarget( float angle_in_deg ) {
   // convert to radians
   float angle_in_rad = radians(angle_in_deg);
   float cur_angle = kine.getTheta();

   // need to turn clockwise
   if (angle_in_rad > 0){
       // if current angle is less than desired, turn a little
       if(cur_angle <= angle_in_rad ){
          motor_R.setMotorPower(-20);
          motor_L.setMotorPower(20);
          cur_angle = kine.getTheta(); //get current angle again
       }
       //else stop moving
       else{
        motor_R.setMotorPower(0);
        motor_L.setMotorPower(0);
       }
   }
   // need to turn anti-clockwise
   else {
       // if current angle is more than desired, turn a little
       if(cur_angle >= angle_in_rad ){
          motor_R.setMotorPower(20);
          motor_L.setMotorPower(-20);
          cur_angle = kine.getTheta();
       }
       //else stop moving
       else{
        motor_R.setMotorPower(0);
        motor_L.setMotorPower(0);
       }
   }
}


// Function to play a beep once at given volume, for delay_ms long
void PlayBeep(int volume, int delay_ms){
  analogWrite(6, volume);
  delay(delay_ms);
  analogWrite(6, 0);
  delay(delay_ms);
}

// Function to flash the built in LED once, for delay_ms long
void FlashLED(int delay_ms){
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(delay_ms); // delay by given time
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(delay_ms);
}

// Function to scale power value based on M
void PowerScaling(float M){  
  float power_right = M * POWER_MAX * (-1);
  float power_left = M * POWER_MAX * (1);

//  motor_R.setMotorPower(power_right);
//  motor_L.setMotorPower(power_left);

//use this in bang bang?  
}
