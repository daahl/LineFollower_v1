#include <Arduino.h>
#include "defines.h"
#include "SparkFun_TB6612.h"
#include "Wire.h"
#include <EEPROM.h>
#include "RemoteXY_gui.h"

// Motor setup
Motor motorLL = Motor(AIN1, AIN2, PWMA, LOFFSET, STBY);
Motor motorRR = Motor(BIN1, BIN2, PWMB, ROFFSET, STBY);
int16_t motorSpeed;
int16_t LLSpeed;
int16_t RRSpeed;

// I2C setup
struct {
  int32_t nearError;
  int32_t midError;
  int32_t farError;
  int32_t BWratio;
} I2CData;

// Memory setup
struct DataStruct {
  int16_t bwThreshold; // -32768 .. +32767
  int16_t BWCurveThr; // -32768 .. +32767, threshold for curve detection
  int16_t NSpeed; // -32768 .. +32767, speed in curves (N) and on straights (F)
  int16_t FSpeed; // -32768 .. +32767
  uint8_t UseCamera;
  float LROffset; // > 1.0, factor that R motor should be faster then L motor
  float NNearP; // P values for near and mid areas, near and far away
  float NMidP;
  float FNearP;
  float FMidP;
};

float PFactor = 10000; // To lower GUI values

DataStruct EEPROMData;
DataStruct GUIData;

bool EEPROM2GUI = true;

// Control setup
float Dfactor = 0; // not implmented yet
float currError = 0;
float lastError = 0;
float dError = 0;
float lastTime = 0;
float currTime = 0;
float turnValue = 0;

// Write GUI settings to I2C
void writeI2C() {
  Wire.beginTransmission((uint8_t)I2C_DEV_ADDR);
  Wire.write(GUIData.bwThreshold);
  Wire.endTransmission(true);
}

// Read data from camera
void readI2C() {
  uint8_t requestI2C = Wire.requestFrom(I2C_DEV_ADDR, 16);

  if (requestI2C){
    uint8_t I2Cbuffer[16];
    Wire.readBytes(I2Cbuffer, 16);

    I2CData.nearError = (int32_t)(I2Cbuffer[0] | 
                                  I2Cbuffer[1] << 8 | 
                                  I2Cbuffer[2] << 16 |
                                  I2Cbuffer[3] << 24);

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

    /*Serial.print("near: " + String(I2CData.nearError));
    Serial.print("\tmid: " + String(I2CData.midError));
    Serial.print("\tfar: " + String(I2CData.farError));
    Serial.println("\tBW: " + String(I2CData.BWratio));*/
  }
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

  EEPROM.begin(EEPROM_SIZE);

  initLEDS();

  blinkLEDS();

  // Must be connected to app to drive
  while (EEPROM2GUI) {

    // Update GUI with last settings
    if (RemoteXY.connect_flag) {
      EEPROM.readBytes(EEPROM_ADDR, &GUIData, EEPROM_SIZE);

      RemoteXY.bwThreshold = GUIData.bwThreshold;
      RemoteXY.BWCurveThr = GUIData.BWCurveThr;
      RemoteXY.LROffset = GUIData.LROffset;
      RemoteXY.UseCamera = GUIData.UseCamera;
      RemoteXY.NSpeed = GUIData.NSpeed;
      RemoteXY.FSpeed = GUIData.FSpeed;
      RemoteXY.NNearP = GUIData.NNearP;
      RemoteXY.NMidP = GUIData.NMidP;
      RemoteXY.FNearP = GUIData.FNearP;
      RemoteXY.FMidP = GUIData.FMidP;

      EEPROM2GUI = false;
      break;
    }

    RemoteXY_delay(100);

  }

  blinkLEDS();

}

void loop() {

  RemoteXY_Handler();

  // Save new GUI settings to EEPROM on request
  if (RemoteXY.EEPROM){
    memcpy(&EEPROMData, &GUIData, EEPROM_SIZE);
    EEPROM.writeBytes(EEPROM_ADDR, &EEPROMData, EEPROM_SIZE);
    EEPROM.commit();
    RemoteXY.EEPROM = 0;
    Serial.println("EEPROM updated");
  }

  // Update values from GUI
  GUIData.bwThreshold = RemoteXY.bwThreshold;
  GUIData.BWCurveThr = RemoteXY.BWCurveThr;
  GUIData.LROffset = RemoteXY.LROffset;
  GUIData.UseCamera = RemoteXY.UseCamera;
  GUIData.NSpeed = RemoteXY.NSpeed;
  GUIData.FSpeed = RemoteXY.FSpeed;
  GUIData.NNearP = RemoteXY.NNearP;
  GUIData.NMidP = RemoteXY.NMidP;
  GUIData.FNearP = RemoteXY.FNearP;
  GUIData.FMidP = RemoteXY.FMidP;

  // Allows to run bot without camera on
  if (RemoteXY.UseCamera){
    // Write values to camera
    writeI2C();

    // Read values from camera
    readI2C();
  

  lastTime = currTime;
  currTime = millis();

  lastError = currError;
  currError = I2CData.midError + I2CData.nearError;
  dError = (currError - lastError) / (currTime - lastTime);

  Serial.println("dError: " + String(dError) + "\tlastError: " + String(lastError) +
                 "\tcurrError: " + String(currError) + "\tlastTime: " + String(lastTime) +
                 "\tcurrTime: " + String(currTime));

  // base case, there is a line in front of the robot, ie semi straight road
  if (I2CData.BWratio > GUIData.BWCurveThr) {
    turnValue = I2CData.nearError * GUIData.FNearP/PFactor + 
                I2CData.midError * GUIData.FMidP/PFactor;
    motorSpeed = GUIData.FSpeed;

  // no line, we're at a curve
  } else {
    turnValue = I2CData.nearError * GUIData.NNearP/PFactor + 
                I2CData.midError * GUIData.NMidP/PFactor;
    motorSpeed = GUIData.NSpeed;
  }

}
else {

  // no camera, use default values
  turnValue = 0;
  motorSpeed = GUIData.FSpeed;

}

  // TODO add constraints
  // TODO try to make it such that turnValue only increases speed, never decreases
  if (RemoteXY.MotorState) {

    LLSpeed = (motorSpeed + (int)turnValue)*GUIData.LROffset;
    RRSpeed = motorSpeed - (int)turnValue;

    LLSpeed = (int)constrain(LLSpeed, -255, 255);
    RRSpeed = (int)constrain(RRSpeed, -255, 255);

    motorLL.drive(LLSpeed);
    motorRR.drive(RRSpeed);

  } else {

    motorLL.brake();
    motorRR.brake();

  }

  // TODO increase this becuase camera needs 110 ms to finish processing
  RemoteXY_delay(80);
}