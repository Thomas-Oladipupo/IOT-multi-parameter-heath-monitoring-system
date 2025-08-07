#include "TemperatureHandler.h"
#include "BluetoothHandler.h" // Include BluetoothHandler to access ESP_BT
#include <Arduino.h>

const int oneWireBus = 4; // GPIO for DS18B20
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

unsigned long currentMillis = 0;

void setupTemperature() {
    sensors.begin();
    currentMillis = millis();
}

void runTemp() {
    if ((millis() - currentMillis) > 2000) {
        sensors.requestTemperatures();
        float temperatureC = sensors.getTempCByIndex(0);
        // Use ESP_BT from BluetoothHandler
        if (BT_cnx) {
        //    ESP_BT.print('E');
            ESP_BT.println(temperatureC);
        }
Serial.println(temperatureC);//debug reasons
        currentMillis = millis();
    }
} 