#include "BluetoothHandler.h" // Include BluetoothHandler to access ESP_BT
#include <Arduino.h>
#include "ECGHandler.h"
#define LO_P 12
#define LO_M 13

void setupECG() {
    pinMode(LO_P , INPUT); // Setup for leads off detection LO +
  pinMode(LO_M , INPUT); // Setup for leads off detection LO -
}

void runECG() {
    if((digitalRead(LO_P) == 1)||(digitalRead(LO_M) == 1)){
    Serial.println('!');
  //  ESP_BT.println('!');
  }
  else{
    // send the value of analog input 0 to serial:
   // Serial.println(analogRead(A0));
    //Do the same for blutooth
    if(BT_cnx){
   //   ESP_BT.print('E');
      ESP_BT.println(analogRead(A0)/4);//ln was here!
   }
  }
}