
#include "Arduino.h"
#include "defines.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "esp_camera.h"        // Include Camera library
#include "EEPROM.h"            // For camera to function
#include "Wire.h"              // For I2C communication
#include "camera.h"

PIDResults pidValues = {0, 0};

// for sending PID values over I2C
union {
  struct {
    int32_t centerError;
    int32_t speedMod;
  } values;
  uint8_t bytes[8];
} I2Cdata;

void onRequestI2C() {

  Serial.print("I2C data requested. Sending PID values: ");
  Serial.println((String)pidValues.centerError + ":" + (String)pidValues.speedMod);

  I2Cdata.values.centerError = pidValues.centerError;
  I2Cdata.values.speedMod = pidValues.speedMod;

  Wire.write(I2Cdata.bytes, 8);
}

void onReceiveI2C(int len) {

  if (serialDebug) {
    Serial.println("I2C data received");
  }

  ledState("blink");

  String i2cMsg = Wire.readString();
  Serial.println("LED state: " + i2cMsg);
  if (i2cMsg == "1"){
    digitalWrite(FLASH_LED, HIGH);
  } 
  else if (i2cMsg == "0"){
    digitalWrite(FLASH_LED, LOW);
  }
  
}

void setup() {

  // set up I2C communication
  Wire.begin(I2C_DEV_ADDR, I2C_SDA, I2C_SCL, I2C_FREQ);
  Wire.onReceive(onReceiveI2C);
  Wire.onRequest(onRequestI2C);

  //disable brownout detector (not sure if needed)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
 
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  Serial.println("LineCam starting...");

  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);

  // initialize camera
  initCamera();

  Serial.println("Parameters set");
  Serial.println("Setup done. LineCam online!");

  // user feedback
  ledState("blink");
  delay(500);
  ledState("on");
  
}

void loop() {

  pidValues = pid();

}
