#include "MAX30102Handler.h"
#include "BluetoothHandler.h" // Include BluetoothHandler to access ESP_BT
#include <Arduino.h>
#include "DisplayHandler.h"
#define MAX_BRIGHTNESS 255

uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
 
uint32_t tsLastReport = 0;  //stores the time the last update was sent to the blynk app

int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value calcualated as per Maxim's algorithm
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid

byte pulseLED = 2; //onboard led on esp32 nodemcu
byte readLED = 19; //Blinks with each data read 

long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute; //stores the BPM as per custom algorithm
int beatAvg = 0, sp02Avg = 0; //stores the average BPM and SPO2 
float ledBlinkFreq; //stores the frequency to blink the pulseLED

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;

MAX30105 particleSensor;
 
void setupMAX30102() {
  Serial.print("Initializing Pulse Oximeter..");
  /*
    // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }  */
    // Initialize sensor
particleSensor.begin(Wire, I2C_SPEED_FAST); //Use default I2C port, 400kHz speed
particleSensor.setup(); //Configure sensor with default settings
particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
   /* 
display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Start the OLED display
display.display();
delay(3000);

*/
  /*The following parameters should be tuned to get the best readings for IR and RED LED. 
   *The perfect values varies depending on your power consumption required, accuracy, ambient light, sensor mounting, etc. 
   *Refer Maxim App Notes to understand how to change these values
   *I got the best readings with these values for my setup. Change after going through the app notes.
   */
 /* byte ledBrightness = 200; //Options: 0=Off to 255=50mA
  byte sampleAverage = 1; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 69; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384
  
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
*/
}
 
void runSpO()
{
  bufferLength = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps
  
  //read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check(); //Check the sensor for new data
  
    redBuffer[i] = particleSensor.getIR();
    irBuffer[i] = particleSensor.getRed();
    particleSensor.nextSample(); //We're finished with this sample so move to next sample
  
    Serial.print(F("red: "));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F("\t ir: "));
    Serial.println(irBuffer[i], DEC);
  }
  
  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  
  //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
  while (1)
  {
    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }
  
    //take 25 sets of samples before calculating the heart rate.
    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor.available() == false) //do we have new data?
        particleSensor.check(); //Check the sensor for new data
    
      digitalWrite(readLED, !digitalRead(readLED)); //Blink onboard LED with every data read
    
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); //We're finished with this sample so move to next sample

      //send samples and calculation result to terminal program through UART
      //Uncomment these statements to view the raw data during calibration of sensor.
      //When uncommented, beatsPerMinute will be slightly off.
      /*Serial.print(F("red: "));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F("\t ir: "));
      Serial.print(irBuffer[i], DEC);
      Serial.print(F("\t HR="));
      Serial.print(heartRate, DEC);
      Serial.print(F("\t"));
      Serial.print(beatAvg, DEC);
      
      Serial.print(F("\t HRvalid="));
      Serial.print(validHeartRate, DEC);
      
      Serial.print(F("\t SPO2="));
      Serial.print(spo2, DEC);
      
      Serial.print(F("\t SPO2Valid="));
      Serial.println(validSPO2, DEC);*/

      long irValue = irBuffer[i];

      //Calculate BPM independent of Maxim Algorithm. 
      if (checkForBeat(irValue) == true)
      {
        //We sensed a beat!
        long delta = millis() - lastBeat;
        lastBeat = millis();
      
        beatsPerMinute = 60 / (delta / 1000.0);
        beatAvg = (beatAvg+beatsPerMinute)/2;

        if(beatAvg != 0)
          ledBlinkFreq = (float)(60.0/beatAvg);
        else
          ledBlinkFreq = 0;
        ledcWriteTone(0, ledBlinkFreq);
      }
      if(millis() - lastBeat > 10000)
      {
        beatsPerMinute = 0;
        beatAvg = (beatAvg+beatsPerMinute)/2;
        
        if(beatAvg != 0)
          ledBlinkFreq = (float)(60.0/beatAvg);
        else
          ledBlinkFreq = 0;
        ledcWriteTone(0, ledBlinkFreq);
      }
    }
  
    //After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
    
    Serial.print(beatAvg, DEC);
    
    Serial.print(F("\t HRvalid="));
    Serial.print(validHeartRate, DEC);
    
    Serial.print(F("\t SPO2="));
    Serial.print( sp02Avg , DEC);
    
    Serial.print(F("\t SPO2Valid="));
    Serial.println(validSPO2, DEC);

    //Calculates average SPO2 to display smooth transitions on Blynk App
    if(validSPO2 == 1 && spo2 < 100 && spo2 > 0)
    {
      sp02Avg = (sp02Avg+spo2)/2;
    }
    else
    {
      spo2 = 0;
      sp02Avg = (sp02Avg+spo2)/2;;
    }
  }
}

/*
void runBPM(){
	long irValue = particleSensor.getIR(); //Reading the IR value it will permit us to know if there's a finger on the sensor or not
//Also detecting a heartbeat
if(irValue > 7000){ //If a finger is detected
display.clearDisplay(); //Clear the display
display.drawBitmap(5, 5, logo2_bmp, 24, 21, WHITE); //Draw the first bmp picture (little heart)
display.setTextSize(2); //Near it display the average BPM you can display the BPM if you want
display.setTextColor(WHITE);
display.setCursor(50,0);
display.println("BPM");
display.setCursor(50,18);
display.println(beatAvg);
display.display();

if (checkForBeat(irValue) == true) //If a heart beat is detected
{
display.clearDisplay(); //Clear the display
display.drawBitmap(0, 0, logo3_bmp, 32, 32, WHITE); //Draw the second picture (bigger heart)
display.setTextSize(2); //And still displays the average BPM
display.setTextColor(WHITE);
display.setCursor(50,0);
display.println("BPM");
display.setCursor(50,18);
display.println(beatAvg);
display.display();
//tone(3,1000); //And tone the buzzer for a 100ms you can reduce it it will be better
//delay(100);
//noTone(3); //Deactivate the buzzer to have the effect of a "bip"
//We sensed a beat!
long delta = millis() - lastBeat; //Measure duration between two beats
lastBeat = millis();

beatsPerMinute = 60 / (delta / 1000.0); //Calculating the BPM

if (beatsPerMinute < 255 && beatsPerMinute > 20) //To calculate the average we strore some values (4) then do some math to calculate the average
{
rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
rateSpot %= RATE_SIZE; //Wrap variable

//Take average of readings
beatAvg = 0;
for (byte x = 0 ; x < RATE_SIZE ; x++)
beatAvg += rates[x];
beatAvg /= RATE_SIZE;
}
}

}
if (irValue < 7000){ //If no finger is detected it inform the user and put the average BPM to 0 or it will be stored for the next measure
beatAvg=0;
display.clearDisplay();
display.setTextSize(1);
display.setTextColor(WHITE);
display.setCursor(30,5);
display.println("Please Place ");
display.setCursor(30,15);
display.println("your finger ");
display.display();
//noTone(3);
}
	}
	
	*/
void runBPM() {
    long irValue = particleSensor.getIR();

    if (irValue > 7000) { // Finger detected
        displayBPM(beatAvg);
//ESP_BT.println(beatAvg);
        if (checkForBeat(irValue)) {
            showHeartBeatAnimation(beatAvg);
         ESP_BT.println(beatAvg);
            long delta = millis() - lastBeat; //Measure duration between two beats
lastBeat = millis();

beatsPerMinute = 60 / (delta / 1000.0); //Calculating the BPM

if (beatsPerMinute < 255 && beatsPerMinute > 20) //To calculate the average we strore some values (4) then do some math to calculate the average
{
rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
rateSpot %= RATE_SIZE; //Wrap variable

//Take average of readings
beatAvg = 0;
for (byte x = 0 ; x < RATE_SIZE ; x++)
beatAvg += rates[x];
beatAvg /= RATE_SIZE;
}
        }
    } else { // No finger detected
        promptFingerPlacement();
    }
}
	/* This code works with MAX30102 + 128x32 OLED i2c + Buzzer and Arduino UNO
* It's displays the Average BPM on the screen, with an animation and a buzzer sound
* everytime a heart pulse is detected
* It's a modified version of the HeartRate library example
* Refer to www.surtrtech.com for more details or SurtrTech YouTube channel
*/

  