/*

  sigfox.h - Library for utilising the sigfox capabilities of
  the bluefox development board by Net Sensors Ltd
  Created by Doug Mahy, January 6, 2018.
  Released into the public domain.

*/

#include "Arduino.h"
#include "Sigfox.h"
#include "SHT3x.h"
#include "WiFi.h"

#define  BATTERY_MONITOR_PIN A6
#define  SIGFOX_TX_PIN 26
#define  SIGFOX_RX_PIN 25
#define  SIGFOX_RESET_PIN 5
#define  LED_PIN 15

HardwareSerial SigfoxSerial(1);

SHT3x Sensor( 0x44,         //Set the address
                SHT3x::Zero,  //Functions will return zeros in case of error
                255,          //If you DID NOT connected RESET pin
                SHT3x::SHT30, //Sensor type
                SHT3x::Single_HighRep_ClockStretch //Low repetability mode
                );

const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data

const float measured = 4.13;
const float reported = 3.78;

//boolean _newData = false;

Sigfox::Sigfox(){
	

  pinMode(SIGFOX_RESET_PIN, OUTPUT);
  pinMode(LED_PIN,INPUT_PULLDOWN);
  analogSetPinAttenuation(BATTERY_MONITOR_PIN, ADC_11db);
  analogReadResolution(12); //12 bits 4095, 11 bits 2047, 10 bits 1024, 9 bits 512
  digitalWrite(SIGFOX_RESET_PIN, HIGH);

}

void Sigfox::begin(){


	WiFi.mode( WIFI_OFF ); // Make sure the WiFi is turned off to save power
	Sensor.SetUpdateInterval(1000);
	Sensor.Begin();
	resetModem();
	SigfoxSerial.begin(9600, SERIAL_8N1, SIGFOX_RX_PIN, SIGFOX_TX_PIN);
	// May be a better idea starting with the sigfox modem asleep to conserve power
	sleep(100);


}

bool Sigfox::sleep(long timeout){

	if (executeATcommand("AT$P=1\r\n",timeout)) {

		if (recievedData == "OK"){
                  //printDebug("Sleeping",recievedData);
				  return true;
		}
	}
	else {
				  //printDebug("Sleep","timeout");
				  return false;
	}

}

bool Sigfox::wakeup(){

   //need to add in return type showing success or failure
   //resetModem();

    printDebug("Initialise the sigfox modem");
    if (executeATcommand("AT\n",200)){
        printDebugValue((String)receivedChars);
    }
    else {
            printDebugValue("Timeout!");
            printDebug("Initialise the sigfox modem");
            if (executeATcommand("AT\n",500)){
                  printDebugValue((String)receivedChars);
            }
            else {
                 printDebugValue("Timeout!");
                 printDebug("Initialise the sigfox modem");
                 if (executeATcommand("AT\n",1000)){
                   printDebugValue((String)receivedChars);
                 }
                 else {
                          printDebugValue("Timeout!");
						  return false;
                 }
            }
    }

    // set modem to full power output
    printDebug("Set sigfox power to fifteen");
    if (executeATcommand("ATS302=15\n",1000)){
       printDebugValue((String)receivedChars);
    }
    else {
       printDebugValue("Timeout!");
	   return false;
    }


	return true;
}

String Sigfox::getPacCode(){

	return getPacCode(1000);

}

String Sigfox::getPacCode(long timeout){

	if (executeATcommand("AT$I=11\r\n",timeout)){
                  printDebug("PAC code",recievedData);
				  return recievedData;
	}
	else {
				  printDebug("PAC code","timeout");
				  return "timeout";
	}

}

String Sigfox::getDeviceId(){

	return getDeviceId(1000);

}

String Sigfox::getDeviceId(long timeout){

	if (executeATcommand("AT$I=10\r\n",timeout)){
                  printDebug("Device Id",recievedData);
				  return recievedData;
	}
	else {
				  printDebug("Device Id","timeout");
				  return "timeout";
	}

}
void Sigfox::sendHexString(String message){

      printDebug("Data for sending", message);

	  SigfoxSerial.print("AT$SF=");
      SigfoxSerial.print(message);
      SigfoxSerial.print("\r\n"); //send carrige return to serial
	  // the modem will complete the trnsfer then go to sleep so we
      // do not need to wait for the Ack as it will waste power.
      sleep(100);

}

void Sigfox::sendByteArray(byte *message){

	  printDebug("Sending sigfox data","OK");

      // in this case we send the data to the sigfox modem and sleep the processor
      // the modem will complete the trnsfer then go to sleep so we
      // do not need to wait for the Ack as it will waste power.
      SigfoxSerial.print("AT$SF=");
      for(int i=0;i<12;i++){  //print bytes to serial
        if (message[i]<0x10) SigfoxSerial.print("0");  //add in leading 0 to hex string
        SigfoxSerial.print( message[i],HEX);
      }
      SigfoxSerial.print("\n"); //send new line return to serial
      sleep(100);

}

String Sigfox::downlinkHexString(String message){

		printDebug("Data for sending", message);

		String ATString = "AT$SF=" + message + ",1\r\n";

		if (executeATcommand(ATString,50000)){
                  printDebug("Recieved Data",recievedData);
				  listenForData(1000);
				  return recievedData;
		}
		else {
				  printDebug("Device Id","timeout");
				  return "timeout";
		}
		sleep(100);

}

void Sigfox::resetModem(){
    digitalWrite(SIGFOX_RESET_PIN, LOW);
    wait(100);
    digitalWrite(SIGFOX_RESET_PIN, HIGH);
    //allows modem time to initialise minimum tested 50ms
    wait(60);
}

bool Sigfox::executeATcommand(String atCommand, int timeout) {

     long sigfoxlast = millis();
     static byte ndx = 0;
     char endMarker = '\n';
     char rc;
     SigfoxSerial.print(atCommand);

      // setup timeout to exit while
      while(millis() - sigfoxlast < timeout){
         while (SigfoxSerial.available() > 0)   {
             rc = SigfoxSerial.read();
             if (rc != endMarker) {
                 receivedChars[ndx] = rc;
                 ndx++;
                 if (ndx >= numChars) {
                    ndx = numChars - 1;
                 }
             }
             else {
               receivedChars[ndx] = '\0'; // terminate the string
               ndx = 0;
			   recievedData = receivedChars;
               return true;
             }
             yield();
          }
          yield();
      }
	  recievedData = receivedChars;
      return false;
}

bool Sigfox::listenForData(int timeout) {

     long sigfoxlast = millis();
     static byte ndx = 0;
     char endMarker = '\n';
     char rc;

      // setup timeout to exit while
      while(millis() - sigfoxlast < timeout){
         while (SigfoxSerial.available() > 0)   {
             rc = SigfoxSerial.read();
             if (rc != endMarker) {
                 receivedChars[ndx] = rc;
                 ndx++;
                 if (ndx >= numChars) {
                    ndx = numChars - 1;
                 }
             }
             else {
               receivedChars[ndx] = '\0'; // terminate the string
               ndx = 0;
			   recievedData = receivedChars;
               return true;
             }
             yield();
          }
          yield();
      }
	  recievedData = receivedChars;
      return false;

}

void Sigfox::wait(long delaytime){
    long delayMillis = millis();
    while (millis() < (delayMillis + delaytime)) {}
}

void Sigfox::printDebug(String key){

#ifdef MY_DEBUG

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
  #endif


}

void Sigfox::printDebug(String key,String val){
  #ifdef MY_DEBUG
      printDebug(key);
      printDebugValue(val);
   #endif
}

void Sigfox::printDebugValue(String val){
  #ifdef MY_DEBUG
     Serial.println(val);
    #ifdef DEBUG_LOG
     logString.concat(val);
     logString.concat("\n");
    #endif

  #endif

}

float Sigfox::getTemperature(){

	Sensor.UpdateData();

	return (float)Sensor.GetTemperature();


}

float Sigfox::getRelHumidity(){

	Sensor.UpdateData();

	return (float)Sensor.GetRelHumidity();


}

float Sigfox::getBatteryVoltage() {

	int aValue = analogRead(BATTERY_MONITOR_PIN);
	float correction = measured/reported;
	float bVoltage = ((3.3/4095.00)*aValue)*2*correction;
	return bVoltage;

}
