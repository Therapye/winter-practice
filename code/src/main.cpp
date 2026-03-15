#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "MechanicalDisplay.h"

MechanicalDisplay display(3, 2);


void setup() {
  Serial.begin(9600);
  display.begin();

}


void loop()
{
  display.updateDigit();

}