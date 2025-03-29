#ifndef CAMERA_H
#define CAMERA_H

#include "Arduino.h"
#include "esp_camera.h"

typedef struct {

    int32_t nearError;
    int32_t midError;
    int32_t farError;
    int32_t BWRatio;

} CamValues;

void init_camera();

void take_photo();

void ledState(String f);

CamValues calculate_cam_values();

#endif