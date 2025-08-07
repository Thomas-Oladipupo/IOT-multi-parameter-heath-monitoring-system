#ifndef MAX30102_HANDLER_H
#define MAX30102_HANDLER_H

#include <MAX30105.h>
#include <Wire.h>
#include "spo2_algorithm.h"
#include "heartRate.h"
#include <Adafruit_GFX.h> //OLED libraries
#include <Adafruit_SSD1306.h>
#include "BluetoothHandler.h" // Include BluetoothHandler to access ESP_BT

extern MAX30105 particleSensor;

void setupMAX30102();
void runSpO();
void runBPM();
#endif