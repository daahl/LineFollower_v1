#include <Arduino.h>
#include "defines.h"
#include "SparkFun_TB6612.h"
#include "Wire.h"

// Motor setup
Motor motorLL = Motor(AIN1, AIN2, PWMA, LOFFSET, STBY);
Motor motorRR = Motor(BIN1, BIN2, PWMB, ROFFSET, STBY);
int speed = 0;

struct {
  int32_t nearError;
  int32_t midError;
  int32_t farError;
  int32_t BWratio;
} I2CData;

uint8_t requestI2C;

float Pfactor = 0.003;
float Dfactor = 0; // not implmented yet
float currError = 0;
float lastError = 0;
float dError = 0;
float lastTime = 0;
float currTime = 0;
float turnValue = 0;

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
uint8_t RemoteXY_CONF[] =   // 96 bytes
  { 255,8,0,2,0,89,0,19,0,0,0,76,105,110,101,67,97,109,0,8,
  1,106,200,1,1,5,0,7,32,52,40,10,118,64,2,26,2,67,32,25,
  40,10,86,2,26,7,32,79,40,10,118,64,2,26,2,7,32,105,40,10,
  110,64,2,26,2,2,129,12,9,85,8,64,17,69,114,114,111,114,44,32,
  66,87,44,32,83,112,101,101,100,44,32,80,118,97,108,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  int16_t bwThreshold = 220; // -32768 .. +32767
  int16_t speed = 30; // -32768 .. +32767
  float Pvalue = 0.001;

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

  initLEDS();

  blinkLEDS();

}

void loop() {

  RemoteXY_Handler();

  
  Pfactor = RemoteXY.Pvalue;
  //Dfactor = RemoteXY.Pvalue; // TEMP

  writeI2C();

  requestI2C = Wire.requestFrom(I2C_DEV_ADDR, 16);

  if (requestI2C){
    uint8_t I2Cbuffer[16];
    Wire.readBytes(I2Cbuffer, 16);

    Serial.print("I2C: ");

    I2CData.nearError = (int32_t)(I2Cbuffer[0] | 
                                  I2Cbuffer[1] << 8 | 
                                  I2Cbuffer[2] << 16 |
                                  I2Cbuffer[3] << 24);

    Serial.print(I2CData.nearError);

    I2CData.midError = (int32_t)(I2Cbuffer[4] |
                                  I2Cbuffer[5] << 8 | 
                                  I2Cbuffer[6] << 16 |
                                  I2Cbuffer[7] << 24);
    
    I2CData.farError = (int32_t)(I2Cbuffer[8] |
                                  I2Cbuffer[9] << 8 | 
                                  I2Cbuffer[10] << 16 |
                                  I2Cbuffer[11] << 24);

    I2CData.BWratio = (int32_t)(I2Cbuffer[12] |
                                  I2Cbuffer[13] << 8 | 
                                  I2Cbuffer[14] << 16 |
                                  I2Cbuffer[15] << 24);
  }

  /*lastTime = currTime;
  currTime = millis();

  lastError = currError;
  currError = I2CData.midError;
  dError = (currError - lastError) / (currTime - lastTime);

  turnValue = currError * Pfactor + dError * Dfactor;*/

  // base case, there is a line in front of the robot, ie semi straight road
  if (abs(I2CData.farError) > 500) {
    turnValue = I2CData.midError * Pfactor;
    speed = RemoteXY.speed;

  // no line, we're at a curve
  } else {
    turnValue = I2CData.nearError * Pfactor * 5;
    speed = RemoteXY.speed * 0.5;
  }

  RemoteXY.centerError = constrain((int16_t)turnValue, -32768, 32767);

  // TODO add constraints
  motorLL.drive(speed + (int)turnValue);
  motorRR.drive(speed - (int)turnValue);

  RemoteXY_delay(50);
}