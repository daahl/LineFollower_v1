#include "Arduino.h"
#include "defines.h"
#include "camera.h"

camera_fb_t *fb = NULL;
camera_config_t config;

// tuneing parameters, look ahead rows
// how many rows ahead of the robot will determine the center of the line. 
// <= fb->height
uint8_t centerLAR = 120; 
// at which row does the center line affect the speed of the robot 
// (ie. slow down before curves), <= fb->height
uint8_t speedLAR = 120;
// threshold for line detection, assume black line on white background, 0-255
// given from GUI through I2C master
extern uint16_t BWthreshold;

void init_camera(){ 

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

    bool serialDebug = false;

    int32_t blacks = 0;
    int32_t whites = 0;

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
                    whites += 1;
                } else {
                    Serial.print("0");
                    blacks += 1;
                }
            }

            Serial.println();

        }


        Serial.println("Blacks: " + (String)blacks);
        Serial.println("Whites: " + (String)whites);
        Serial.println();

    }

    esp_camera_fb_return(fb); 
};

void ledState(String f){

    bool serialDebug = false;

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

CamValues calculate_cam_values(){

    CamValues values = {0, 0, 0, 0};

    bool serialDebug = false;

    // take photo
    take_photo();

    // calculate centering based on a 3x3 grid
    // near error, middle error, and far error, can have different PID values
    // since th sensor is flipped, x0 and y0 are at the bottom right corner
    int nearLimit = (fb->height/3)*fb->width;
    int midLimit = (2*fb->height/3)*fb->width;
    int rightLim = fb->width/3; // read image from right to left
    int leftLim = 2*fb->width/3;

    // reset values
    values = {0, 0, 0, 0};

    for (int row = 1; row <= fb->height; row++){

        int rightPixelStart = (row-1)*fb->width;
        int pixelStop = rightPixelStart + rightLim;

        // count right and left columns at the same time
        for (int pixel = rightPixelStart; pixel < pixelStop; pixel++){

            // near error
            if (pixel < nearLimit){

                if (fb->buf[pixel] < BWthreshold){
                    values.nearError += 1;
                }
                if (fb->buf[pixel + leftLim] < BWthreshold){
                    values.nearError -= 1;
                }

            }
            // mid error
            else if (nearLimit <= pixel && pixel < midLimit){

                if (fb->buf[pixel] < BWthreshold){
                    values.midError += 1;
                }
                if (fb->buf[pixel + leftLim] < BWthreshold){
                    values.midError -= 1;
                }

            }
            // far error
            else if (midLimit <= pixel){

                if (fb->buf[pixel] < BWthreshold){
                    values.farError += 1;
                }
                if (fb->buf[pixel + leftLim] < BWthreshold){
                    values.farError -= 1;
                }

            }

        }

    }

    if (serialDebug){
        Serial.println(String(values.nearError) + " || " 
                        + String(values.midError) + " || "
                        + String(values.farError));
    }

    return values;

};