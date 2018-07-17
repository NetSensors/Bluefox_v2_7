/*
  
  sigfox.h - Library example utilising the sigfox capabilities of 
  the bluefox development board by Net Sensors Ltd
  Created by Doug Mahy, January 6, 2018.
  Released into the public domain.
  
*/
#define MY_DEBUG

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_deep_sleep.h"
#include "Wire.h"
#include "Sigfox.h"
#include "BluefoxUtils.h"

#include "SparkFunLIS3DH.h"

#define OFFSET_MAX 4
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  30      /* Time ESP32 will go to sleep (in seconds) */

//create an instance of the Sigfox library/class
Sigfox sigfox;
BluefoxUtils utils;

RTC_DATA_ATTR byte message[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
RTC_DATA_ATTR int offset = 0; 

LIS3DH myIMU; //Default constructor is I2C, addr 0x19.

void setup() {

    pinMode(0,OUTPUT);
    
    // configure the esp 32
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    //esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    digitalWrite(0,LOW);
    
    
    if (offset > OFFSET_MAX) offset=0;

    
    Serial.begin(115200); Serial.println(); Serial.println("Starting ....");  //start up the serial port so you can see what is happening
    
    myIMU.settings.adcEnabled = 0;
    myIMU.settings.tempEnabled = 0;
    myIMU.settings.accelSampleRate = 1;  //Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
    myIMU.settings.accelRange = 4;      //Max G force readable.  Can be: 2, 4, 8, 16
    myIMU.settings.xAccelEnabled = 1;
    myIMU.settings.yAccelEnabled = 1;
    myIMU.settings.zAccelEnabled = 1;
    //Call .begin() to configure the IMU
    myIMU.begin();
      
    // modem starts powered down to save power until it is needed so needs to be reset/turned on before use.  
    // The if else clause can be used to check the success of the reset as reset returns true for success and false for fail
    sigfox.begin();   // initialise instance of sigfox class modem
    
    #ifdef MY_DEBUG
      
      utils.printDebug("Wakeup Sigfox");
      if (sigfox.wakeup()){
       utils.printDebugValue("success");
      }
      else{
      utils.printDebugValue("fail"); 
      }  
      
      // gets the pac code a function parameter can be used to set the timeout in number of milliseconds
      // return type is String containing pac code or "timeout" if pac code cannot be retrieved.
      utils.printDebug("PAC code",sigfox.getPacCode());
      
      // as above but for the device id
      utils.printDebug("Device Id",sigfox.getDeviceId(2000));

           
      sigfox.sleep(100);
    
    #endif

  // as we have to send bytes 0xE0 and 0xFF rather than normal numbers like 10.47 and 55.32 we have to mess about with 
  // the numbers to encode them in a way we can use bytes to send them. In this case im going to do something simple and 
  // split them up by sending the stuff before the decimal point in the first byte and the stuff after the decimal point 
  // in the second byte. We can reassemble them at the other end so they look like normal decimal numbers.
  String voltage = (String)sigfox.getBatteryVoltage();
  int integralVoltage = getIntegral(voltage);
  int fractionalVoltage  = getFractional(voltage);
  
  String temperature = (String)sigfox.getTemperature();
  int integralTemperature = getIntegral(temperature);
  int fractionalTemperature  = getFractional(temperature);

  String relHumidity = (String)sigfox.getRelHumidity();
  int integralHumidity = getIntegral(relHumidity);
  int fractionalHumidity = getFractional(relHumidity);

  // useful for debugging 
  #ifdef MY_DEBUG
    utils.printDebug("Temperature",temperature);
    utils.printDebug("Humidity",relHumidity);
    utils.printDebug("Voltage",voltage);
  #endif

  // example commented out below where we can send a byte array instead of a string   
  message[0 + offset] = (byte)integralTemperature;
  message[1 + offset] = (byte)fractionalTemperature;
  message[2 + offset] = (byte)integralHumidity;
  message[3 + offset] = (byte)fractionalHumidity;
  message[10] = (byte)integralVoltage;
  message[11] = (byte)fractionalVoltage;

  #ifdef MY_DEBUG
    utils.printDebug("0 byte",(String)message[0]);
    utils.printDebug("1 byte",(String)message[1]);
    utils.printDebug("2 byte",(String)message[2]);
    utils.printDebug("3 byte",(String)message[3]);
    utils.printDebug("4 byte",(String)message[4]);
    utils.printDebug("5 byte",(String)message[5]);
    utils.printDebug("6 byte",(String)message[6]);
    utils.printDebug("7 byte",(String)message[7]);
    utils.printDebug("8 byte",(String)message[8]);
    utils.printDebug("9 byte",(String)message[9]);
    utils.printDebug("10 byte",(String)message[10]);
    utils.printDebug("11 byte",(String)message[11]);
  #endif
  
  if (offset == OFFSET_MAX) {
    sigfox.wakeup();
    sigfox.sendByteArray(message);
    utils.printDebug("Sending","OK");
    sigfox.sleep(1);
  }

      utils.wait(2000);
      
     //Get all parameters
      Serial.print("\nAccelerometer:\n");
      Serial.print(" X = ");
      Serial.println(myIMU.readFloatAccelX(), 4);
      Serial.print(" Y = ");
      Serial.println(myIMU.readFloatAccelY(), 4);
      Serial.print(" Z = ");
      Serial.println(myIMU.readFloatAccelZ(), 4);

  offset = offset + 4;
  esp_deep_sleep_start();

}

void loop() {
  // put your main code here, to run repeatedly:
}


int getIntegral(String strfloat){
     return strfloat.substring(0, strfloat.indexOf(".")).toInt();
}
int getFractional(String strfloat){
  return strfloat.substring(strfloat.indexOf(".")+1,strfloat.length()).toInt();
}
