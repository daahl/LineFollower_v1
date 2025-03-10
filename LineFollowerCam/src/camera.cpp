#include "Arduino.h"
#include "defines.h"
#include "camera.h"

camera_fb_t *fb = NULL;
camera_config_t config;

// tuneing parameters, look ahead rows
// how many rows ahead of the robot will determine the center of the line. <= fb->height
uint8_t centerLAR = 120; 
// at which row does the center line affect the speed of the robot (ie. slow down before curves), <= fb->height
uint8_t speedLAR = 120;
// threshold for line detection, assume black line on white background, 0-255
uint8_t BWthreshold = 180;

void initCamera(){ 

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

    pinMode(FLASH_LED, OUTPUT); // flash LED

    // Init Camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }else{
        Serial.println("Camera init success");
    }

};

void take_photo(){
  
    fb = esp_camera_fb_get();

    if (serialDebug) {

        if(!fb) {
            Serial.println("Camera capture failed");
            return;
        };

        // Set to skip # of rows and columns in printing
        int rowColSkip = 1;

        for (int k = 0; k < (fb->len - fb->width); k = k + fb->width*rowColSkip){
        
            for (int i = 0; i < fb->height; i = i + 1*rowColSkip){

                if (fb->buf[k+i] > BWthreshold){
                    Serial.print("1");
                } else {
                    Serial.print("0");
                }
            }

            Serial.println();

        }

    }

    esp_camera_fb_return(fb); 
};

void ledState(String f){
    if (f == "on"){
        digitalWrite(FLASH_LED, HIGH);
    } 
    else if (f == "off"){
        digitalWrite(FLASH_LED, LOW);
    }
    else if (f == "blink"){
        digitalWrite(FLASH_LED, HIGH);
        delay(500);
        digitalWrite(FLASH_LED, LOW);
    }
    else {
        if (serialDebug) {
            Serial.println("Invalid command");
        }   
    }

    if (serialDebug) {
        Serial.println("LED state: " + f);
    }

};

PIDResults pid(){

    PIDResults results = {0, 0};

    // take photo
    take_photo();

    // calculate centering
    // remember that camera is mounted upside down!!
    // pixel count = width*height
    // black = 0, white = 255
    int startRow = fb->height - 1;                // Bottom row (closest to robot)
    int endRow = fb->height - centerLAR;          // Stop at centerLAR rows from bottom
    if (endRow < 0) endRow = 0;    

    for (int row = startRow; row > endRow; row = row - fb->width){

        // Calculate buffer position for this row
        int rowStartPos = row * fb->width;

        for (int col = 0; col < fb->width; col++){
           
            // Determine if pixel is on left or right side
            int LR = (col < fb->width/2) ? -1 : 1;

            // only count black pixels
            if (fb->buf[rowStartPos + col] < BWthreshold){
                results.centerError += LR;
            }

        }

    }

    if (serialDebug){
        Serial.println("Error: " + String(results.centerError));
    }

    return results;

};