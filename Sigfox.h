/*
  
  sigfox.h - Library for utilising the sigfox capabilities of 
  the bluefox development board by Net Sensors Ltd
  Created by Doug Mahy, January 6, 2018.
  Released into the public domain.
  
*/

#ifndef Sigfox_h
#define Sigfox_h
#include "Arduino.h"



class Sigfox
{
  public:
    
	Sigfox();
    
	void begin();
	
	bool wakeup();
	
	bool sleep(long timeout);
	
	String getPacCode();
	
	String getPacCode(long timeout);
	
	String getDeviceId();
	
	String getDeviceId(long timeout);
	
	float getTemperature();
	
	float getRelHumidity();
	
	float getBatteryVoltage();
	
	void sendHexString(String message);
	
	void sendByteArray(byte message[12]);
	
	String downlinkHexString(String message);
		
	String recievedData;
	
	bool executeATcommand(String atCommand, int timeout);
	
	bool listenForData(int timeout);
	
  private:
   		
	void resetModem();
	
	void wait(long delaytime);
	
	void printDebug(String key);
	
	void printDebug(String key,String val);
	
	void printDebugValue(String val);
		 
};

#endif
