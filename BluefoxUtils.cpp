/*

  BluefoxUtils.h - Library for utilising the sigfox capabilities of
  the bluefox development board by Net Sensors Ltd
  Created by Doug Mahy, January 6, 2018.
  Released into the public domain.

*/



#include "Arduino.h"
#include "BluefoxUtils.h"
#include "WiFi.h"

#define LED 15


BluefoxUtils::BluefoxUtils(){
		//pinMode(LED, OUTPUT);

}

void BluefoxUtils::wait(long delaytime){
    long delayMillis = millis();
    while (millis() < (delayMillis + delaytime)) {}
}

void BluefoxUtils::printDebug(String key){

	
		  //check length of key and value is less than 30 charachters if more then truncate else pad with charachters.

		  int keylength = 30;

		  if (key.length() > keylength){
			key.remove(keylength);
		  }
		  else{
			 for (int i = key.length(); i < keylength; i++){
				  key.concat(" ");
			 }
		  }
		  key.concat(": ");
		  Serial.print(key);
  
  
	

  
  
  
    #ifdef DEBUG_LOG
      logString.concat(key);
    #endif



}

void BluefoxUtils::printDebug(String key,String val){
	
		  printDebug(key);
		  printDebugValue(val);
	
}

void BluefoxUtils::printDebugValue(String val){


	
     Serial.println(val);
	
    #ifdef DEBUG_LOG
     logString.concat(val);
     logString.concat("\n");
    #endif



}

void BluefoxUtils::flash(long flashOnMillis, long flashOffMillis, int repeat){

  pinMode(LED, OUTPUT);
  for (int i=0;i<repeat;i++){

    digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    wait(flashOnMillis);
    digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
    wait(flashOffMillis);

  }



}

void BluefoxUtils::getScanResults(struct scanResult* results, int maxResults) {

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  wait(100);

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks(false,false);

    //utils.printDebugValue((String)n + " networks found");

  if (n >= maxResults){

    n = maxResults - 1;

    //utils.printDebug("More than max basesations found");
    //utils.printDebug("Using max value",(String)maxResults);

  }

  int o = n;
  int loops = 0;

                // sort by RSSI
                int indices[n];
                int skip[n];

                String ssid;

                for (int i = 0; i < n; i++) {
                  indices[i] = i;
                }

                // CONFIG
                bool sortRSSI   = true; // sort aps by RSSI
                bool removeDups = true; // remove dup aps ( forces sort )
                bool printAPs   = true; // print found aps

                // --------
                if(removeDups || sortRSSI){
                  for (int i = 0; i < n; i++) {
                    for (int j = i + 1; j < n; j++) {
                      if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
                        loops++;
                        //int temp = indices[j];
                        //indices[j] = indices[i];
                        //indices[i] = temp;
                        std::swap(indices[i], indices[j]);
                        std::swap(skip[i], skip[j]);
                      }
                    }
                  }
                }

                if(removeDups){
                  for (int i = 0; i < n; i++) {
                    if(indices[i] == -1){
                      --o;
                      continue;
                    }
                    ssid = WiFi.SSID(indices[i]);
                    for (int j = i + 1; j < n; j++) {
                      loops++;
                      if (ssid == WiFi.SSID(indices[j])) {
                        indices[j] = -1;
                      }
                    }
                  }
                }

                //utils.printDebug("Networks no duplicates",(String)o);

                for (int i = 0; i < maxResults; ++i) results[i].index = -1; //clear array ready for new data
                for (int i = 0; i < n; ++i)
                {

                 if (indices[i] != -1){

                    results[i].rssi = (String)WiFi.RSSI(indices[i]);
                    results[i].bssid = (String)WiFi.BSSIDstr(indices[i]);
                    results[i].ssid = (String)WiFi.SSID(indices[i]);
                    results[i].index = indices[i];

                  }
                }

}
