#include <Arduino.h>
#include "defines.h"
#include "SparkFun_TB6612.h"
#include "Wire.h"
#include <EEPROM.h>
#include "RemoteXY_gui.h"

// Motor setup
Motor motorLL = Motor(AIN1, AIN2, PWMA, LOFFSET, STBY);
Motor motorRR = Motor(BIN1, BIN2, PWMB, ROFFSET, STBY);
int speed = 0;

// I2C setup
struct {
  int32_t nearError;
  int32_t midError;
  int32_t farError;
  int32_t BWratio;
} I2CData;

uint8_t requestI2C;

// Memory setup
struct DataStruct {
  int16_t bwThreshold; // -32768 .. +32767
  int16_t speed; // -32768 .. +32767
  float Pvalue;
};

DataStruct EEPROMData;
DataStruct GUIData;

bool EEPROM2GUI = true;

// Control setup
float Pfactor = 0.003;
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

  EEPROM.begin(EEPROM_SIZE);

  initLEDS();

  blinkLEDS();

  // Must be connected to app to drive
  while (EEPROM2GUI) {

    // Update GUI with last settings
    if (RemoteXY.connect_flag) {
      EEPROM.readBytes(EEPROM_ADDR, &GUIData, EEPROM_SIZE);

      RemoteXY.bwThreshold = GUIData.bwThreshold;
      RemoteXY.speed = GUIData.speed;
      RemoteXY.Pvalue = GUIData.Pvalue;

      EEPROM2GUI = false;
      break;
    }

    RemoteXY_delay(100);

  }

  blinkLEDS();

}

void loop() {

  // Save new GUI settings to EEPROM on request
  if (RemoteXY.EEPROM){
    memcpy(&EEPROMData, &GUIData, EEPROM_SIZE);
    EEPROM.writeBytes(EEPROM_ADDR, &EEPROMData, EEPROM_SIZE);
    EEPROM.commit();
    RemoteXY.EEPROM = 0;
    Serial.println("EEPROM updated");
  }

  GUIData.bwThreshold = RemoteXY.bwThreshold;

  Serial.println("bwThreshold: " + (String)GUIData.bwThreshold);


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
  // TODO try to make it such that turnValue only increases speed, never decreases
  if (RemoteXY.MotorState) {

    motorLL.drive(speed + (int)turnValue);
    motorRR.drive(speed - (int)turnValue);

  }
  

  RemoteXY_delay(50);
}