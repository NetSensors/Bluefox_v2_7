/*

  sigfox.h - Library for utilising the sigfox capabilities of
  the bluefox development board by Net Sensors Ltd
  Created by Doug Mahy, January 6, 2018.
  Released into the public domain.

*/



struct scanResult
{
  String bssid;
  String rssi;
  String ssid;
  int index;
};

#ifndef BluefoxUtils_h
#define BluefoxUtils_h
#include "Arduino.h"

class BluefoxUtils
{
  public:

	BluefoxUtils();
	void wait(long delaytime);
	void printDebug(String key);
	void printDebug(String key,String val);
	void printDebugValue(String val);
	void flash(long flashOnMillis, long flashOffMillis, int repeat);
	void getScanResults(struct scanResult* results, int maxResults);

  private:

};

#endif
