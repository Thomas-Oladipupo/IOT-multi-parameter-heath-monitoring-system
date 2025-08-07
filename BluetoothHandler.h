#ifndef BLUETOOTH_HANDLER_H
#define BLUETOOTH_HANDLER_H

#include <BluetoothSerial.h>

extern BluetoothSerial ESP_BT; // Declare the Bluetooth object globally
extern boolean BT_cnx;         // Declare the global connection status variable

void setupBluetooth();
void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

#endif