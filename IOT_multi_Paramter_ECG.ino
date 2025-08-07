#include <Arduino.h>
#include "BluetoothHandler.h"
#include "TemperatureHandler.h"
#include "MAX30102Handler.h"
#include "ECGHandler.h"
#include "BloodPressureHandler.h"
#include "DisplayHandler.h"
#include  "PressureMonitoringExampleHandler.h"
#define LED_BUILTIN 2 // Pin with LED to turn on when BT connected
String Mode="";
String receivedData = ""; // String to store incoming data
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
	setupDisplay();
    setupBluetooth();
   setupECG();
    setupTemperature();
   setupMAX30102();
   //setupPressureDialogue();
}

void loop() {// only one function should be run at a time so that it does not interrupt the measurement in other blocks, i will use touch button click and hold to determine which block runs
	
unsigned long initMicro=millis();
    	initMicro=millis();
        


  /* if (ESP_BT.available()){
        Serial.write(ESP_BT.read());  
        Serial.print("new Mode:");
            Serial.println(Mode);
        }
    Mode= ESP_BT.readString();
    */
  /*  char incomingChar = ESP_BT.read();
    if (incomingChar != '\n'){
      receivedData += String(incomingChar);
    }
    else{
    	Mode= receivedData;
            Serial.print("new Mode");
            Serial.println(Mode);
      receivedData = "";
    } 
      Mode= SerialBT.readString();
    Serial.write(incomingChar);  
  }
        */
  /*
  while (ESP_BT.available()) {           // Check if data is available
    char incomingChar = ESP_BT.read();   // Read one character
    if (incomingChar == '\n') {          // Check if end of the word (newline)
      Serial.print("Received word: ");
      Serial.println(receivedData);      // Print the full word
      Mode=receivedData;
     receivedData = "";                 // Clear the buffer for the next word
    } else {
      receivedData += incomingChar;      // Append character to the buffer
    }
  } 
  */
  
  /*
  if(Mode=="ECG"){
  	runECG();// works fine
  	receivedData="";// reset for ecg only
  	}
  	else if(Mode=="BPM"){
  	runBPM();// works fine
  	}
  	else if(Mode=="BT"){
  	runTemp();// works fine
  	}
  	*/
//runPressureDialogue();
//runPressureRaw();
   // runECG();// works fine
    //runBPM();// works fine
    runTemp();// works fine
   // runSpO();
   Serial.print("elapsed time: ");//debug reasons
Serial.print((initMicro-millis()));//debug reasons
Serial.println(" milliseconds ");//

    delay(2); // To prevent overload
}  