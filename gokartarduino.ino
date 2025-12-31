/*
Created by MustBePerfect

Code for the remote operation of my treadmill go kart using an Arduino UNO R4 Wifi. 
*/

#include "pwm.h"

PwmOut pwm(D3);

// Brake system that time based and switch based system reference to avoid conflicts
enum ActuatorCmd {
  ACT_STOP,
  ACT_EXTEND,
  ACT_RETRACT
};

ActuatorCmd actuatorCmd = ACT_STOP;

// Pin assignments
const int rcPin = 2;
const int switchPin = 4;
const int fwdPin = 5;
const int revPin = 6;
const int steeringPin = 7;
const int steeringFwdPin = 9;
const int steeringRevPin = 10;

// PWM values for throttle and switches
const unsigned long NEUTRAL = 1507;
const unsigned long MAX_FORWARD = 976;
const unsigned long MAX_REVERSE = 2040;
const unsigned long DEADBAND = 23;

// PWM values for each brake stage
const unsigned brakeStageTwo = 1660;
const unsigned brakeStageThree = 1790;
const unsigned brakeStageFour = 1920;

// Extra brake data
float brakePercent;
float lastBrakePercent = 0.0;
unsigned long brakeStartTime = 0;
unsigned long brakeRunTime = 0;
bool braking = false;
bool releasing = false;
const float fullBrakeTime = 3.0;

// Variable input values
unsigned long pulseWidth;
unsigned long switchPulse;
unsigned long steeringPulse;

float dutyCycle;


void setup() {

  Serial.begin(9600);
  pwm.begin(20.0f, 0.0f);

  // Pin INPUT/OUTPUT declarations
  pinMode(rcPin, INPUT);
  pinMode(switchPin, INPUT);
  pinMode(steeringPin, INPUT);

  pinMode(fwdPin, OUTPUT);
  pinMode(revPin, OUTPUT);

  pinMode(steeringFwdPin, OUTPUT);
  pinMode(steeringRevPin, OUTPUT);

  // Pin assignments
  digitalWrite(fwdPin, LOW);
  digitalWrite(revPin, LOW);

  digitalWrite(steeringFwdPin, LOW);
  digitalWrite(steeringRevPin, LOW);
}

void loop() {

  // Get PWM values
  pulseWidth = pulseIn(rcPin, HIGH, 25000);
  switchPulse = pulseIn(switchPin, HIGH, 25000);
  steeringPulse = pulseIn(steeringPin, HIGH, 25000);
  Serial.println(steeringPulse);

  // Throttle logic
  if (pulseWidth < NEUTRAL - DEADBAND) {
    dutyCycle = (float)(NEUTRAL - pulseWidth) / (NEUTRAL - MAX_FORWARD) * 100.0f;
    dutyCycle = constrain(dutyCycle, 0, 100);
    //brakePercent = 0.0;
  } else if (pulseWidth > NEUTRAL + DEADBAND && pulseWidth < brakeStageTwo) {
    //brakePercent = 25.0;
  } else if (pulseWidth > brakeStageTwo && pulseWidth < brakeStageThree) {
    //brakePercent = 50.0;
  } else if (pulseWidth > brakeStageThree && pulseWidth < brakeStageFour) {
    //brakePercent = 75.0;
  } else if (pulseWidth > brakeStageFour) {
    //brakePercent = 100.0;
  } else {
    dutyCycle = 0;
  }

  // Steering Logic
  if (steeringPulse < NEUTRAL - DEADBAND) {
    digitalWrite(steeringFwdPin, HIGH);
    digitalWrite(steeringRevPin, LOW);
  } else if (steeringPulse > NEUTRAL + DEADBAND) {
    digitalWrite(steeringFwdPin, LOW);
    digitalWrite(steeringRevPin, HIGH);
  } else {
    digitalWrite(steeringFwdPin, LOW);
    digitalWrite(steeringRevPin, LOW);
  }


  // Time based brake logic. Unstable. 

  /*

  unsigned long now = millis();

  unsigned long targetBrakeTime =
      (unsigned long)((brakePercent / 100.0f) * fullBrakeTime * 1000.0f);

  if (brakePercent > 0.0f && !braking && brakePercent != lastBrakePercent) {
    braking = true;
    releasing = false;
    brakeStartTime = now;
    brakeRunTime = targetBrakeTime;
  }

  if (braking) {
    actuatorCmd = ACT_EXTEND;

    if (now - brakeStartTime >= brakeRunTime) {
      braking = false;
      actuatorCmd = ACT_STOP;
    }
  }

  if (brakePercent == 0.0f && lastBrakePercent > 0.0f && !releasing) {
    releasing = true;
    braking = false;

    unsigned long elapsedBrakeTime = now - brakeStartTime;

    brakeStartTime = now;
    brakeRunTime = elapsedBrakeTime;
  }

  if (releasing) {
    actuatorCmd = ACT_RETRACT;

    if (now - brakeStartTime >= brakeRunTime) {
      releasing = false;
      actuatorCmd = ACT_STOP;
    }
  }

  lastBrakePercent = brakePercent;

  */

  // Brake switch logic
  int switchState = 0;

  if (switchPulse < 1300) {
    switchState = 2;
  } 
  else if (switchPulse > 1800) {
    switchState = 0;
  } 
  else {
    switchState = 1;
  }

  // Prevents conflicts with time based system. Redudant if time system is disabled. 
  if (!braking && !releasing) {
    if (switchState == 0) {
      actuatorCmd = ACT_EXTEND;
    } else if (switchState == 2) {
      actuatorCmd = ACT_RETRACT;
    } else {
      actuatorCmd = ACT_STOP;
    }
  }
  
  // Actual brake trigger that both systems reference
  switch (actuatorCmd) {
  case ACT_EXTEND:
    digitalWrite(fwdPin, HIGH);
    digitalWrite(revPin, LOW);
    break;

  case ACT_RETRACT:
    digitalWrite(fwdPin, LOW);
    digitalWrite(revPin, HIGH);
    break;

  default:
    digitalWrite(fwdPin, LOW);
    digitalWrite(revPin, LOW);
    break;
  }

  pwm.pulse_perc(dutyCycle);

  delay(200);

}









