#include <Arduino.h>
#include "defines.h"

void blinkLEDS() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  delay(1000);
}

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

}

void loop() {
  
  blinkLEDS();

}