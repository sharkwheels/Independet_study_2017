/********************
weather servos 1.0
Nadine L
June 2017
References: https://github.com/brendanmatkin/BlindSpot/tree/master/_96_servo_wave
******************/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <elapsedMillis.h>


Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);    // initialize the servo drivers

#define SERVOMIN  200     // (out of 4096) pulse length
#define SERVOMAX  600     // (out of 4096) pulse length
int servonum = 0;         // servo counter
int degrees = 0;          // initialize. 180 and 75 are closest to 180 and 90 for these servos
bool ud = true;        // up/down boolean (servo position alternator)
bool end = false;
bool servosActive = false;

elapsedMillis timerWave;
int waveSpeed = 500;      // speed of the servo wave. Lower is faster (keep it above 1). >~200 is one servo at a time without overlap

elapsedMillis timerRest;
int servoInterval = 10000; // 

void setup() {
  Serial.begin(9600);
  pwm.begin();           // start wire communication with servo controllers
  pwm.setPWMFreq(60);    // leave at 60 (update frequency for servos) 
  delay(100);
  for(int i; i < 7; i++){
    pwm.setPWM(i,0,SERVOMIN);
  }
  timerRest = 0;
}

void loop() {
  
  if(servosActive){
      runServos();
  } else {
    for(int i; i < 7; i++){
      pwm.setPWM(i,0,SERVOMIN);
    }
    servonum = 0;
    timerWave = 0; 
  }

  if (timerRest > servoInterval) {
    timerRest -= servoInterval;
    servosActive = !servosActive;
  }

}


void runServos(){
  if (servonum > 7) {
    servonum = 7;
    end = true;            
  }else if(servonum < 0){
    servonum = 0;
    end = false;
  }
  if(end){
    ud = false; // change direction..of servo
  }else{
    ud = true; 
  }

  if (ud == true) {       // up/down boolean (servo position alternator)
    degrees = 180;        // anything between 0 and 180
  } else {
    degrees = 10;         // anything between 0 and 180
  }

  int pulselength = map(degrees,0,180,SERVOMIN,SERVOMAX);    // re-map degrees to pulselength for the servos (so we can think in degrees)
  
  if (servonum <= 7) {                                      // divide the servo number out among the controllers
    pwm.setPWM(servonum, 0, pulselength);                   // (each servo on each controller is numbered 0-15)
    if (timerWave > waveSpeed) {
      timerWave -= waveSpeed; //reset the timer
      if(end){
        servonum--;
      }else{
        servonum ++; 
      }
      Serial.println(servonum);
      Serial.print("ud: ");
      Serial.println(ud);    
    }
  }

}

