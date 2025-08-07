#include "BluetoothHandler.h" // Include BluetoothHandler to access ESP_BT
#include <Arduino.h>
#include  "PressureMonitoringExampleHandler.h"
const int ADC_PIN = 39;        // ADC pin
const int PUMP_FB_ADC_PIN = 33;        // ADC pin
const int CONTROL_PIN = 14;    // Pin that toggles HIGH and LOW after Button 1 press
const int BUTTON1_PIN = 25;//to start taking of readings 
const int INTERRUPT_READY = 25; // Pin for the interrupt (capture readiness)
const int INTERRUPT_PRINT = 26; // Pin to print captured values
const int LED_INDICATOR = 27;   // Single LED for all processes
const int MAX_VALUES = 2000;    // Maximum number of values to log
  
int capturedValues[MAX_VALUES]; // Array to store captured values
int captureIndex = 0;           // Index for the array
volatile bool capturing = false; // Flag to indicate if we are capturing values
bool printing = false;           // Flag for printing values
unsigned long lastDebounceTimeReady = 0; // Debounce timer for Button 1
unsigned long lastDebounceTimePrint = 0; // Debounce timer for Button 2
unsigned long debounceDelay = 1000;        // Debounce delay
unsigned long ledTimer = 0;              // Timer for LED blinking
unsigned int maximumPressureAdc=0;

 int lastValue = 0;           // Store the last ADC value
 int initialNormalValue = 0; // Value before the spike
bool spikeDetected = false; // Track if a spike has been detected
 
  #define MAX_PREVIOUS_VALUES 20 // Maximum number of previous values
#define THRESHOLD 10            // Threshold for peak detection

int previousValues[MAX_PREVIOUS_VALUES]; // Array to store previous values
int previousCount = 0;                   // Number of previous values stored
int peakValues[MAX_PREVIOUS_VALUES];     // Array to store peak values
int peakCount = 0;                       // Number of peak values stored

/**
 * Compares a new value with previous values to check if it is greater than any of the previous values by a threshold.
 * If true, stores the new value in the peakValues array.
 * 
 * @param newValue The new value to check.
 */
// ISR to start capturing
void IRAM_ATTR handleReadyInterrupt() {
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTimeReady) > debounceDelay) {
    capturing = true; // Start capturing
    Serial.println("Capture process started.");
    lastDebounceTimeReady = currentTime;
  }
}

// ISR to print captured values
void IRAM_ATTR handlePrintInterrupt() {
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTimePrint) > debounceDelay) {
    printing = true; // Set the printing flag
  //  detachInterrupt(digitalPinToInterrupt(INTERRUPT_READY)); // Detach interrupt
    Serial.println("Capture process complete. Press Button 1 to restart.");
    lastDebounceTimePrint = currentTime;
  }
}
 
void setupPressureDialogue() {
  
  analogReadResolution(12); // Configure ADC resolution if needed
  pinMode(ADC_PIN, INPUT);
  pinMode(CONTROL_PIN, OUTPUT); // Configure the CONTROL_PIN as output
  digitalWrite(CONTROL_PIN, HIGH);
  //pinMode(INTERRUPT_READY, INPUT_PULLUP); // Configure the ready interrupt pin
  pinMode(BUTTON1_PIN, INPUT_PULLUP); // Configure the pin
  pinMode(INTERRUPT_PRINT, INPUT_PULLUP); // Configure the print interrupt pin
  pinMode(LED_INDICATOR, OUTPUT); // Configure LED indicator

  // Attach interrupt for printing
 // attachInterrupt(digitalPinToInterrupt(INTERRUPT_PRINT), handlePrintInterrupt, RISING);

  Serial.println("Press Button 1 to start...");
}

void runPressureDialogue() {
  // Simulate Button 1 behavior
  if (digitalRead(BUTTON1_PIN) == LOW) {
    simulateButton1Press();
  }

  // Handle capturing process
  if (capturing) {
    blinkLED(1000); // Slow blinking during capture
    CAPTURE();
  }

  if(!printing){
  if (digitalRead(INTERRUPT_PRINT) == LOW) {
   printing = true; // Set the printing flag
   Serial.println("----- Captured peak Values -----");
 }
  }
  // Handle printing process
  if (printing) {
    blinkLED(200); // Fast blinking during printing
    PRINT_CAPTURED_VALUES();
  }

 // delay(100); // Delay for ADC stability
}


// Function to capture values between a spike and return to normal
void CAPTURE() {
  	unsigned long N=0;
     for(int i=0; i< 30; i++){// iterate to give a more consistent reading
     N += analogRead(ADC_PIN);
     delayMicroseconds(10);
     	}
     	N /=30;
     	
    int currentValue = N;
    checkPeak(currentValue); //
    // below is to test run
if (captureIndex < MAX_VALUES) {
        capturedValues[captureIndex++] = currentValue; // Log the current value
      } else {
        Serial.println("Capture buffer full! Consider increasing MAX_VALUES.");
        capturing = false; // Stop capturing to prevent overflow
      }
      delay(10);
    /*
    // Detect a spike
    if (!spikeDetected && ( currentValue > (lastValue + THRESHOLD) )) {
      spikeDetected = true; // Spike detected
      initialNormalValue = lastValue; // Save the normal value before the spike
     // captureIndex = 0; // Reset capture index not sure if this is needed
      Serial.println("Spike detected! Capturing values...");
    }

    // Capture values if a spike has been detected
    if (spikeDetected) {
      if (captureIndex < MAX_VALUES) {
        capturedValues[captureIndex++] = currentValue; // Log the current value
      } else {
        Serial.println("Capture buffer full! Consider increasing MAX_VALUES.");
        capturing = false; // Stop capturing to prevent overflow
      }

      // Check if the value has returned to the initial normal level
      if (currentValue <= initialNormalValue) {
        Serial.println("Returned to normal level. Stopping capture.");
        spikeDetected = false; // Stop spike detection
        capturing = false; // Stop capturing
        Serial.println("Capture complete.");
  digitalWrite(LED_INDICATOR, LOW); // Turn off LED after capturing
  digitalWrite(CONTROL_PIN, LOW);// simulate button press to turn off pressure monitor 
  delay(200); // Short pulse
  digitalWrite(CONTROL_PIN, HIGH);
      }
    }
*/
    // Update lastValue for the next iteration
    lastValue = currentValue;

  
  
}

// Function to print all captured values with a 100ms delay
void PRINT_CAPTURED_VALUES() {
  /* static int keepPrinting=0;
  if( keepPrinting < captureIndex) {
    Serial.println(capturedValues[keepPrinting]);
    keepPrinting++;
    delay(10); // Delay between each value
  }
  if( keepPrinting >= captureIndex) {
  printing = false;
  keepPrinting=0;
    captureIndex = 0;          // Reset the buffer
    digitalWrite(LED_INDICATOR, LOW); // Turn off LED after printing
  Serial.println("----- End of Captured Values -----"); 
  } */
  printArray(peakValues, peakCount, "Peak Values");
}

// Function to blink LED with a given interval
void blinkLED(unsigned long interval) {
  unsigned long currentTime = millis();
  if (currentTime - ledTimer >= interval) {
    ledTimer = currentTime;
    digitalWrite(LED_INDICATOR, !digitalRead(LED_INDICATOR)); // Toggle LED state
  }
}

// Function to simulate Button 1 behavior
void simulateButton1Press() {// works well
  digitalWrite(CONTROL_PIN, LOW);
  delay(200); // Short pulse
  digitalWrite(CONTROL_PIN, HIGH);

  Serial.println("CONTROL_PIN toggled HIGH->LOW. Waiting 8 seconds...");
  delay(8000); // Wait for 8 seconds
  Serial.println("Waiting for control pin to go off");
  // Attach interrupt for capturing readiness
  //attachInterrupt(digitalPinToInterrupt(INTERRUPT_READY), handleReadyInterrupt, FALLING);
  //Serial.println("Capture readiness interrupt attached.");
  //delay(5000); // Wait for 5 seconds
  unsigned long N=0;
     for(int i=0; i< 100; i++){// iterate to give a more consistent reading
     N += analogRead(PUMP_FB_ADC_PIN);
     delayMicroseconds(10);
     	}
     	N /=100;
  while(N > 600){// continue to read until the voltage 
  //from pwm output drops to signify end of pump and take the maximum pressure reading 
  	N=0;
  	for(int i=0; i< 100; i++){// iterate to give a more consistent reading
     N += analogRead(PUMP_FB_ADC_PIN);
     delayMicroseconds(10);
     	}
     	N /=100;
  	}
  	
  	unsigned long MP=0;// max pressure adc
     for(int i=0; i< 30; i++){// iterate to give a more consistent reading
     MP += analogRead(ADC_PIN);
     delayMicroseconds(10);
     	}
     	MP /=30;
     	
    maximumPressureAdc = MP;
    lastValue= maximumPressureAdc;// last value initial before capturing begins 
    capturing = true; // Start capturing
    Serial.print("Maximum pressure adc is: ");
    Serial.println(MP);
    Serial.print("Maximum approximate pressure is: ");
    Serial.println(mapValue(MP));
    delay(1000);// temporary 
    Serial.println("Capture process started.");
    
  }
  
  
void checkPeak(int newValue) {
    bool isPeak = false;

    // Compare the new value with all previous values
    for (int i = 0; i < previousCount; i++) {
        if (newValue - previousValues[i] > THRESHOLD) {
            isPeak = true;
            break;
        }
    }

    // If the new value is a peak, add it to the peakValues array
    if (isPeak && peakCount < MAX_PREVIOUS_VALUES) {
        peakValues[peakCount++] = mapValue(newValue);// return and save calculated pressure 
    }

    // Add the new value to the previousValues array
    if (previousCount < MAX_PREVIOUS_VALUES) {
        previousValues[previousCount++] = newValue;
    } else {
        // Shift previous values to make room for the new value
        for (int i = 1; i < MAX_PREVIOUS_VALUES; i++) {
            previousValues[i - 1] = previousValues[i];
        }
        previousValues[MAX_PREVIOUS_VALUES - 1] = newValue;
    }
}

/**
 * Prints the contents of an array to the Serial Monitor.
 * 
 * @param arr The array to print.
 * @param count The number of elements in the array.
 * @param label The label for the array (e.g., "Previous Values").
 */
void printArray(int arr[], int count, const char* label) {
    Serial.print(label);
    Serial.print(": ");
    for (int i = 0; i < count; i++) {
        Serial.print(arr[i]);
        Serial.println(" ");
    }
    Serial.println();
}

// Function to map input integer values based on the slope and intercept
int mapValue(int inputValue) {
	if (inputValue>=300){
		float slope = 0.129;      // Replace with your calculated slope
    float intercept = -36.197; // Replace with your calculated intercept
    return round((slope * inputValue) + intercept);
		}
    else{
    	return 0;
    	}
}