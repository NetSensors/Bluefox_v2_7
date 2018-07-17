/*
  
  sigfox.h - Library example utilising the sigfox capabilities of 
  the bluefox development board by Net Sensors Ltd
  Created by Doug Mahy, January 6, 2018.
  Released into the public domain.
  
*/

//#define MY_DEBUG
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Sigfox.h"

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  86400       /* Time ESP32 will go to sleep (in seconds) */

//create an instance of the Sigfox library/class
Sigfox sigfox;

void setup() {

   // configure the esp 32
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);


  pinMode(0,OUTPUT);
  digitalWrite(0,HIGH);
    
  Serial.begin(115200); Serial.println(); Serial.println("Starting ....");  //start up the serial port so you can see what is happening
 
  sigfox.begin();   // initialise instance of sigfox class modem 

  // modem starts powered down to save power until it is needed so needs to be reset/turned on before use.  
  // The if else clause can be used to check the success of the reset as reset returns true for success and false for fail
  
  if (sigfox.wakeup()){
    Serial.print("Reset     : ");Serial.println("success");
  }
  else{
    Serial.print("Reset     : ");Serial.println("fail");
  }  

  
  
    // gets the pac code a function parameter can be used to set the timeout in number of milliseconds
  // return type is String containing pac code or "timeout" if pac code cannot be retrieved.
  Serial.print("PAC code  : ");Serial.println(sigfox.getPacCode());

  // as above but for the device id
  Serial.print  ("Device Id : ");Serial.println(sigfox.getDeviceId(2000));

  // string we are going to send, hex chars only allowed must be even number no "half" bytes allowed
  String hexString = "F00DDEADBEEFC0FFEE";

  Serial.print("Sending   : "); Serial.println(hexString);

  
  // Sends the hex string then waits for downlink response either prints response or error message
  // usually a timeout if no response recieved
  //Serial.print("Result    : ");Serial.println(sigfox.downlinkHexString(hexString));

  String strResult = sigfox.downlinkHexString(hexString);
  Serial.print("Result    : ");Serial.println(strResult);

  char hex1[3]; 
  char hex2[3]; 
  strResult.substring(21,23).toCharArray(hex1, 3);
  strResult.substring(24,26).toCharArray(hex2, 3);
  int x  = x2i(hex1) << 8;
  x =  x | x2i(hex2);
  word result = 0xFFFF0000 | x;
  float s = 0-(float)(~(result - 1)); // easy becaus we know its always 0
  Serial.print("Result ");Serial.println(s);

  sigfox.sleep(100);
  // goes to sleep to save battery life, will tx once a day or when reset button pressed 
  esp_deep_sleep_start();

}

void loop() {
  // put your main code here, to run repeatedly:
}

int x2i(char *s) 
{
 int x = 0;
 for(;;) {
   char c = *s;
   if (c >= '0' && c <= '9') {
      x *= 16;
      x += c - '0'; 
   }
   else if (c >= 'A' && c <= 'F') {
      x *= 16;
      x += (c - 'A') + 10; 
   }
   else break;
   s++;
 }
 return x;
}

