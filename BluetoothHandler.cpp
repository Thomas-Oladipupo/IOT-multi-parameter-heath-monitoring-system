#include "BluetoothHandler.h"
#include <Arduino.h>

 #define LED_BUILTIN 2

BluetoothSerial ESP_BT;
boolean BT_cnx = false;

void setupBluetooth() {
    ESP_BT.register_callback(callback);
    if (!ESP_BT.begin("ESP32_ECG")) {
        Serial.println("An error occurred initializing Bluetooth");
    } else {
        Serial.println("Bluetooth initialized... Bluetooth Device is Ready to Pair...");
    }
}

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    if (event == ESP_SPP_SRV_OPEN_EVT) {
        Serial.println("Client Connected");
        digitalWrite(LED_BUILTIN, HIGH);
        BT_cnx = true;
    }

    if (event == ESP_SPP_CLOSE_EVT) {
        Serial.println("Client disconnected");
        digitalWrite(LED_BUILTIN, LOW);
        BT_cnx = false;
        ESP.restart();
    }
}