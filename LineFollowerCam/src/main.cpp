#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>

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
  { 255,4,0,4,0,89,0,19,0,0,0,76,105,110,101,67,97,109,0,8,
  1,106,200,1,1,5,0,2,30,125,44,22,0,2,26,31,31,79,78,0,
  79,70,70,0,7,32,52,40,10,118,64,2,26,2,67,7,24,40,10,86,
  2,26,67,58,24,40,10,86,2,26,12,32,81,40,10,193,30,26,115,110,
  97,105,108,0,110,111,114,109,97,108,0,102,97,115,116,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t motorsState; // =1 if switch ON and =0 if OFF
  int16_t bwThreshold; // -32768 .. +32767
  uint8_t speedState; // from 0 to 3

    // output variables
  int16_t pixelValueLow; // -32768 .. +32767
  int16_t pixelValueHigh; // -32768 .. +32767

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

// define the number of bytes you want to access
#define EEPROM_SIZE 1

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

int pictureNumber = 0;

camera_fb_t *fb = NULL;

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

  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;

  Serial.println("Buffer size: " + String(fb->len));
  Serial.println("Image width: " + String(fb->width));
  Serial.println("Image height: " + String(fb->height));

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

void setup() {

  RemoteXY_Init (); 

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  pinMode(4, OUTPUT); // flash LED
 
  RemoteXY_delay(1000);
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  Serial.println("Terminal open");
  RemoteXY_delay(1000);
  
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

  Serial.println("Parameters set");

  //config.frame_size = FRAMESIZE_SXGA;
  config.frame_size = FRAMESIZE_240X240;
  //config.frame_size = FRAMESIZE_96X96;
  config.fb_count = 1;
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }else{
    Serial.println("Camera init success");
  }
  
  Serial.println("Setup done...");
}

void loop() {
  RemoteXY_Handler();

  take_photo(fb);
  RemoteXY_delay(100);
}
