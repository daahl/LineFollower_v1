
#include "Arduino.h"
#include "defines.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "esp_camera.h"        // Include Camera library
#include "EEPROM.h"            // For camera to function
#include "Wire.h"              // For I2C communication
#include "camera.h"

int16_t BWthreshold = 220;

CamValues camValues = {0, 0, 0, 0};

// for sending PID values over I2C
const int I2C_DATA_SIZE = 16; // 4*int32 entries in the struct

union {
  struct {
    int32_t nearError;
    int32_t midError;
    int32_t farError;
    int32_t BWRatio;
  } values;
  uint8_t bytes[I2C_DATA_SIZE];
} I2Cdata;

void on_request_I2C() {

  bool serialDebug = false;

  I2Cdata.values.nearError = camValues.nearError;
  I2Cdata.values.midError = camValues.midError;
  I2Cdata.values.farError = camValues.farError;
  I2Cdata.values.BWRatio = camValues.BWRatio;

  if (serialDebug) {
    Serial.println("I2C data: " + (String)I2Cdata.values.nearError + ":" + (String)I2Cdata.values.midError + ":" + (String)I2Cdata.values.farError + ":" + (String)I2Cdata.values.BWRatio);
  }

  Wire.write(I2Cdata.bytes, I2C_DATA_SIZE);
}

void on_receive_I2C(int len) {

  bool serialDebug = false;

  int16_t i2cMsg = Wire.read();

  BWthreshold = i2cMsg;

  if (serialDebug) {
    Serial.println("I2C Msg: " + (String)i2cMsg);
  }
  
  
}

void setup() {

  // set up I2C communication
  Wire.begin(I2C_DEV_ADDR, I2C_SDA, I2C_SCL, I2C_FREQ);
  Wire.onReceive(on_receive_I2C);
  Wire.onRequest(on_request_I2C);

  //disable brownout detector (not sure if needed)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
 
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  Serial.println("LineCam starting...");

  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);

  // initialize camera
  init_camera();

  Serial.println("Parameters set");
  Serial.println("Setup done. LineCam online!");

  // user feedback
  ledState("blink");
  delay(500);
  ledState("on");
  
}

void loop() {

  camValues = calculate_cam_values();
  //delay(3000);

}
