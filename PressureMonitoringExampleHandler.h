#ifndef PRESSUREMONITORINGEXAMPLE_HANDLER_H
#define PRESSUREMONITORINGEXAMPLE_HANDLER_H

void setupPressureDialogue();
void runPressureDialogue();
void IRAM_ATTR handleReadyInterrupt();
void IRAM_ATTR handlePrintInterrupt();
void simulateButton1Press();
void CAPTURE();
void PRINT_CAPTURED_VALUES();
void blinkLED(unsigned long interval); 
void checkPeak(int newValue);
int mapValue(int inputValue);
void printArray(int arr[], int count, const char* label);
#endif