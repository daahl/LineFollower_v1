#ifndef CAMERA_H
#define CAMERA_H

#include "Arduino.h"
#include "esp_camera.h"

typedef struct {

    int32_t centerError;
    int32_t speedMod;

} PIDResults;

void initCamera();

void take_photo();

void ledState(String f);

PIDResults pid();

#endif