#include <Arduino.h>
#include "defines.h"
#include "SparkFun_TB6612.h"
#include "Wire.h"

// Motor setup
Motor motorLL = Motor(AIN1, AIN2, PWMA, LOFFSET, STBY);
Motor motorRR = Motor(BIN1, BIN2, PWMB, ROFFSET, STBY);
int speed = 0;
int turn = 0;

int32_t centerError = 0;
int32_t speedMod = 0;

void writeI2C() {
  Serial.println("Writing I2C");
  Wire.beginTransmission((uint8_t)I2C_DEV_ADDR);
  Wire.printf("1");
  Wire.endTransmission(true);
  delay(2000);
  Wire.beginTransmission((uint8_t)I2C_DEV_ADDR);
  Wire.printf("0");
  delay(2000);
  Wire.endTransmission(true);
  Serial.println("Writing I2C done");
}

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
  Serial.println("Blinking LEDs");
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  delay(1000);
  Serial.println("Blinking LEDs done");
}

void setup() {

  Serial.begin(115200);
  Serial.println("LineBot online!");

  Wire.begin();

  initLEDS();

}

void loop() {
  
  blinkLEDS();

  //testMotors();

  //writeI2C();

  uint8_t requestI2C = Wire.requestFrom(I2C_DEV_ADDR, 8);

  if (requestI2C){
    uint8_t I2Cbuffer[8];
    Wire.readBytes(I2Cbuffer, 8);

    // Convert first 4 bytes to centerError (big-endian)
    centerError = ((int32_t)I2Cbuffer[0]) | 
                 ((int32_t)I2Cbuffer[1] << 8) | 
                 ((int32_t)I2Cbuffer[2] << 16) | 
                 (int32_t)I2Cbuffer[3] << 24;

    // Convert last 4 bytes to speedMod (big-endian)
    speedMod = ((int32_t)I2Cbuffer[4]) | 
               ((int32_t)I2Cbuffer[5] << 8) | 
               ((int32_t)I2Cbuffer[6] << 16) | 
               (int32_t)I2Cbuffer[7] << 24;

  }
  
  Serial.println("Received: " + (String)centerError + ":" + (String)speedMod);
}