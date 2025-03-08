
#include "Arduino.h"
#include "defines.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "esp_camera.h"        // Include Camera library
#include <EEPROM.h>            // For camera to function
#include <Wire.h>              // For I2C communication

int pictureNumber = 0;

camera_fb_t *fb = NULL;

void updateMinMax(camera_fb_t *fb, unsigned int arrayLength, int16_t *min, int16_t *max){
  /*
  * Function to update the min and max values of the array
  * @param fb: camera_fb_t pointer
  * @param arrayLength: length of the array
  * @param min: pointer to the min value
  * @param max: pointer to the max value
  */

  for (int i = 0; i < arrayLength; i++){
    if (fb->buf[i] < *min){
      *min = fb->buf[i];
    }

    if (fb->buf[i] > *max){
      *max = fb->buf[i];
    }
  }

}

void take_photo(camera_fb_t *fb){
  // Take Picture with Camera
  //digitalWrite(4, HIGH);
  fb = esp_camera_fb_get(); 
  //digitalWrite(4, LOW); 

  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }else{
    Serial.println("Photo taken");
  }

  // Update GUI
  //updateMinMax(fb, fb->len, &RemoteXY.pixelValueLow, &RemoteXY.pixelValueHigh);

  pictureNumber = EEPROM.read(0) + 1;

  Serial.println("Buffer size: " + String(fb->len));
  Serial.println("Image width: " + String(fb->width));
  Serial.println("Image height: " + String(fb->height));

  // Set to skip # of rows and columns in printing
  int rowColSkip = 2;

  for (int k = 0; k < (fb->len - fb->width); k = k + fb->width*rowColSkip){
    
    for (int i = 0; i < fb->height; i = i + 1*rowColSkip){
      if (fb->buf[k+i] > 127){
        Serial.print("1");
      } else {
        Serial.print("0");
      }
    }

    Serial.println();
  }

  esp_camera_fb_return(fb); 
}

void receiveI2C(int len) {
  Serial.println("I2C data received");
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

  Wire.begin(I2C_DEV_ADDR, I2C_SDA, I2C_SCL, I2C_FREQ);
  //Wire.begin((uint8_t)I2C_DEV_ADDR, (uint8_t)I2C_SDA, (uint8_t)I2C_SCL);
  Wire.onReceive(receiveI2C);

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  pinMode(FLASH_LED, OUTPUT); // flash LED
 
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  Serial.println("LineCam online!");
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE; 
  config.fb_count = 1;

  //config.frame_size = FRAMESIZE_SXGA;
  config.frame_size = FRAMESIZE_240X240;
  //config.frame_size = FRAMESIZE_96X96;

  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);

  Serial.println("Parameters set");
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }else{
    Serial.println("Camera init success");
  }
  
  Serial.println("Setup done");
}

void loop() {

  //take_photo(fb);
  delay(100);
}
