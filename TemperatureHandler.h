#ifndef TEMPERATURE_HANDLER_H
#define TEMPERATURE_HANDLER_H

#include <DallasTemperature.h>
#include <OneWire.h>

extern DallasTemperature sensors;

void setupTemperature();
void runTemp();

#endif