/*

 Simple Bluefox blink example

*/

#include "BluefoxUtils.h"

//create an instance of the utils library/class
BluefoxUtils utils;

void setup() {

}

void loop() {
  
  //utils.flash(ON millis, OFF millis, Repeat)
  utils.flash(10, 50, 10);
  delay(500);
  
}
