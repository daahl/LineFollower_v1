#include <Arduino.h>
#include "defines.h"
#include "SparkFun_TB6612.h"
#include "Wire.h"

// Motor setup
Motor motorLL = Motor(AIN1, AIN2, PWMA, LOFFSET, STBY);
Motor motorRR = Motor(BIN1, BIN2, PWMB, ROFFSET, STBY);
int speed = 20;

int32_t centerError = 0;
float speedMod = 0;

float Pfactor = 0.001;
float turn = 0;
#define historyLength 10
int turnHistory[historyLength];
float turnAvg = 0;

uint8_t requestI2C;

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG    

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__WIFI_POINT

#include <WiFi.h>

// RemoteXY connection settings 
#define REMOTEXY_WIFI_SSID "LineCam"
#define REMOTEXY_WIFI_PASSWORD "LineCam!"
#define REMOTEXY_SERVER_PORT 6377


#include <RemoteXY.h>

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 88 bytes
  { 255,4,0,2,0,81,0,19,0,0,0,76,105,110,101,67,97,109,0,8,
  1,106,200,1,1,4,0,2,30,125,44,22,0,2,26,31,31,79,78,0,
  79,70,70,0,7,32,52,40,10,118,64,2,26,2,12,32,81,40,10,193,
  30,26,115,110,97,105,108,0,110,111,114,109,97,108,0,102,97,115,116,0,
  67,32,25,40,10,86,2,26 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t motorsState; // =1 if switch ON and =0 if OFF
  int16_t bwThreshold; // -32768 .. +32767
  uint8_t speedState; // from 0 to 3

    // output variables
  int16_t centerError; // -32768 .. +32767

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

// Write GUI settings to I2C
void writeI2C() {
  Wire.beginTransmission((uint8_t)I2C_DEV_ADDR);
  Wire.write(RemoteXY.bwThreshold);
  Wire.endTransmission(true);
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
  RemoteXY_delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  RemoteXY_delay(1000);
  Serial.println("Blinking LEDs done");
}

void setup() {

  RemoteXY_Init(); 

  Serial.begin(115200);
  Serial.println("LineBot online!");

  Wire.begin();

  for (int i = 0; i < historyLength; i++) {
    turnHistory[i] = 0;
  }

  initLEDS();

  blinkLEDS();

}

void loop() {

  RemoteXY_Handler();

  writeI2C();

  requestI2C = Wire.requestFrom(I2C_DEV_ADDR, 8);

  if (requestI2C){
    uint8_t I2Cbuffer[8];
    Wire.readBytes(I2Cbuffer, 8);

    // Convert first 4 bytes to centerError (big-endian)
    centerError = ((int32_t)I2Cbuffer[0]) | 
                 ((int32_t)I2Cbuffer[1] << 8) | 
                 ((int32_t)I2Cbuffer[2] << 16) | 
                 ((int32_t)I2Cbuffer[3] << 24);

    // Convert last 4 bytes to speedMod (big-endian)
    speedMod = ((int32_t)I2Cbuffer[4]) | 
               ((int32_t)I2Cbuffer[5] << 8) | 
               ((int32_t)I2Cbuffer[6] << 16) | 
               ((int32_t)I2Cbuffer[7] << 24);

  }
  
  //Serial.println("Received: " + (String)centerError);

  turn = centerError*Pfactor;

  turnAvg = 0;

  for (int i = historyLength - 1; i > 0; i--) {
    turnHistory[i] = turnHistory[i - 1];
    turnAvg += turnHistory[i];
  }

  turnHistory[0] = turn;
  turnAvg += turnHistory[0];
  turnAvg /= historyLength;

  RemoteXY.centerError = constrain((int16_t)turnAvg, -32768, 32767);

  motorLL.drive(speed - (int)turnAvg);
  motorRR.drive(speed + (int)turnAvg);

  RemoteXY_delay(50);
}