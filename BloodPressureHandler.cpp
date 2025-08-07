#include "BluetoothHandler.h" // Include BluetoothHandler to access ESP_BT
#include <Arduino.h>

void runPressureRaw(){
	if(analogRead(39)> 200){// pin VN
  	if(BT_cnx){
      
     unsigned long N=0;
     for(int i=0; i< 50; i++){// iterate to give a more consistent reading
     N += analogRead(39);
     delayMicroseconds(10);
     	}
     	N /=50;
      ESP_BT.print('E');
      ESP_BT.println(N-400);// to remove the 400mV baseline in adc reading
   
   }   
 }
}
