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
int8_t lastTurnDirection[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // -1 = left, 1 = right

// I2C setup
struct {
  int32_t nearError;
  int32_t midError;
  int32_t farError;
  int32_t BWratio;
} I2CData;

// Memory setup
struct DataStruct {

    // input variables
    int16_t bwThreshold; // -32768 .. +32767
    int16_t BWCurveThr; // -32768 .. +32767
    float LROffset; // > 1.0, factor that R motor should be faster then L motor
    int16_t NSpeed; // -32768 .. +32767
    int16_t FSpeed; // -32768 .. +32767
    int16_t NNearP; // -32768 .. +32767
    int16_t NMidP; // -32768 .. +32767
    int16_t FNearP; // -32768 .. +32767
    int16_t FMidP; // -32768 .. +32767
    uint8_t UseCamera; // =1 if switch ON and =0 if OFF
    float NearD; // D values for curves
    float FarD; // D values for straights
};

float PFactor = 10000; // To lower GUI values

DataStruct EEPROMData;
DataStruct GUIData;

bool EEPROM2GUI = true;

// Control setup
float Dfactor = 0.1; // not implmented yet
float currError = 0;
float lastError = 0;
float dError = 0;
float lastTime = 0;
float currTime = 0;
float newTurnValue = 0;
float turnValue[3] = {0 , 0 , 0};

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

void updateLastTurnDirection(int8_t newDirection) {
  for (int i = 0; i < 8; i++) {
      lastTurnDirection[i] = lastTurnDirection[i + 1];
  }
  lastTurnDirection[8] = newDirection;
}

int8_t getLastTurnDirection() {
  int8_t sum = 0;

  // Sum all the values in the lastTurnDirection array
  for (int i = 0; i <= 8; i++) {
      sum += lastTurnDirection[i];
  }

  if (sum > 0) {
    return 1; // right
  } else if (sum < 0) {
    return -1; // left
  } else {
    return 0; // straight
  }
}

void updateTurnValue(float newValue) {
  // Shift all elements to the left
  for (int i = 0; i < 2; i++) {
      turnValue[i] = turnValue[i + 1];
  }
  // Add the new direction to the end of the array
  turnValue[2] = newValue;
}

float getTurnValue() {
  float sum = 0;

  // Sum all the values in the lastTurnDirection array
  for (int i = 0; i < 3; i++) {
      sum += turnValue[i];
  }

  //return sum / 3;
  return turnValue[2]; // test, only use the last value
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
      RemoteXY.NearD = GUIData.NearD;
      RemoteXY.FarD = GUIData.FarD;

      EEPROM2GUI = false;
      break;
    }

    RemoteXY_delay(100);

  }

  blinkLEDS();

}

void loop() {

  loopStart:

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
  GUIData.NearD = RemoteXY.NearD;
  GUIData.FarD = RemoteXY.FarD;

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

  /*Serial.println("dError: " + String(dError) + "\tlastError: " + String(lastError) +
                 "\tcurrError: " + String(currError) + "\tlastTime: " + String(lastTime) +
                 "\tcurrTime: " + String(currTime));*/

  // base case, there is a line in front of the robot, ie semi straight road
  if (I2CData.BWratio > GUIData.BWCurveThr) {
    newTurnValue = I2CData.nearError * GUIData.FNearP/PFactor + 
                I2CData.midError * GUIData.FMidP/PFactor;
    newTurnValue = newTurnValue + dError * GUIData.FarD;
    updateTurnValue(newTurnValue);
    motorSpeed = GUIData.FSpeed;

  // we see little black ahead, we're at a curve
  } else if (I2CData.BWratio < GUIData.BWCurveThr &&
             I2CData.BWratio > GUIData.BWCurveThr/8) {
    newTurnValue = I2CData.nearError * GUIData.NNearP/PFactor + 
                I2CData.midError * GUIData.NMidP/PFactor;
    newTurnValue = newTurnValue + dError * GUIData.NearD;
    updateTurnValue(newTurnValue);
    motorSpeed = GUIData.NSpeed;
  // we see nothing, we're off track, keep turning in the last direction
  } else if (I2CData.BWratio < GUIData.BWCurveThr/8) {

    if (RemoteXY.MotorState) {

      LLSpeed = GUIData.FSpeed * GUIData.LROffset;
      RRSpeed = GUIData.FSpeed;

      LLSpeed = (int)constrain(LLSpeed, -255, 255);
      RRSpeed = (int)constrain(RRSpeed, -255, 255);

      motorLL.drive(-LLSpeed);
      motorRR.drive(-RRSpeed);

      RemoteXY_delay(20);
      goto loopStart;
    }
  }

}
else {

  // no camera, use default values
  updateTurnValue(0);
  updateTurnValue(0);
  updateTurnValue(0);

  motorSpeed = GUIData.FSpeed;

}

  // TODO add constraints
  // TODO try to make it such that turnValue only increases speed, never decreases
  if (RemoteXY.MotorState) {

    LLSpeed = (motorSpeed + (int)getTurnValue())*GUIData.LROffset;
    RRSpeed = motorSpeed - (int)getTurnValue();

    LLSpeed = (int)constrain(LLSpeed, -255, 255);
    RRSpeed = (int)constrain(RRSpeed, -255, 255);

    motorLL.drive(LLSpeed);
    motorRR.drive(RRSpeed);

    if (getTurnValue() > 0) {
      updateLastTurnDirection(1); // right
    } else {
      updateLastTurnDirection(-1); // left
    }

  } else {

    motorLL.brake();
    motorRR.brake();

  }

  Serial.println(getTurnValue());

  // TODO increase this becuase camera needs 110 ms to finish processing
  RemoteXY_delay(80);
}