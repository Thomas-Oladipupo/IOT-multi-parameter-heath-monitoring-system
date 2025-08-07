#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Adafruit_SSD1306.h>
#include <Wire.h>

// OLED Display Constants
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Declare the Adafruit_SSD1306 object for the display
extern Adafruit_SSD1306 display;

// Function Declarations
void setupDisplay();
void showHeartBeatAnimation(int beatAvg);
void promptFingerPlacement();
void displayBPM(int beatAvg);

#endif // DISPLAY_HANDLER_H