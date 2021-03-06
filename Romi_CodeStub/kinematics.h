#ifndef _KINEMATICS_H
#define _KINEMATICS_H

#define PI 3.14

const float RADIUS = 35.00;
const float L = 70.0; // distance from centre of romi to a wheel

// for 1 revolution of wheel -> 1440 counts
const float MM_PER_COUNT = ((2*PI) / 1440.0) * RADIUS; // dist travelled (in mm) for 1 count
// this^ should be 0.15mm...


class kinematics_c {
  public:
      // Function Prototypes
      kinematics_c();   // constructor 
      void update();
      float getX();
      float getY();
      float getTheta();
      float calcAngle();

   private:
      float x;
      float y;
      float theta; //in radians
  
      float prev_e0;
      float prev_e1;
};

// Constructor; initialize all variables to 0.0
kinematics_c::kinematics_c() {
  x = 0.0;
  y = 0.0;
  theta = 0.0;
  prev_e0 = 0.0;
  prev_e1 = 0.0;  
}

// Routine to execute the update to kinematics 
void kinematics_c::update() {
  // Calculate the change in encoder count since the last update
  float change_in_e0 = count_e0 - prev_e0;
  float change_in_e1 = count_e1 - prev_e1;
  prev_e0 = count_e0;
  prev_e1 = count_e1;
  
  // Convert this change to an appropriate unit (mm)
  float dist_L = change_in_e0 * MM_PER_COUNT; //mm
  float dist_R = change_in_e1 * MM_PER_COUNT; //mm

  // mean change in encoder count
  float d = (dist_L + dist_R)/2.0;
  
  x = x + d * cos(theta);
  y = y + d * sin(theta);
  theta = theta + ((dist_L - dist_R)/(2.0*L));

}

// Returns the angle in degrees
float kinematics_c::calcAngle(){
  float angle =  (theta / (2 * PI) * 360 );
  return angle;
}

float kinematics_c::getX(){
  return x;
}

float kinematics_c::getY(){
  return y;
}

float kinematics_c::getTheta(){
  return theta;
}

#endif
