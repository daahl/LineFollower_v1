#include <Arduino.h>
#include "defines.h"
#include "SparkFun_TB6612.h"

// Motor setup
Motor motorLL = Motor(AIN1, AIN2, PWMA, LOFFSET, STBY);
Motor motorRR = Motor(BIN1, BIN2, PWMB, ROFFSET, STBY);
int speed = 0;
int turn = 0;

void testMotors() {
  motorLL.drive(255);
  motorRR.drive(255);
  delay(1000);
  motorLL.drive(-255);
  motorRR.drive(-255);
  delay(1000);
  motorLL.brake();
  motorRR.brake();
  delay(1000);
}

void driveMotors(){
  motorLL.drive(speed + turn);
  motorRR.drive(speed - turn);
}

void initLEDS() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
}

void blinkLEDS() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  delay(1000);
}

void setup() {

  initLEDS();

}

void loop() {
  
  blinkLEDS();

  testMotors();

}