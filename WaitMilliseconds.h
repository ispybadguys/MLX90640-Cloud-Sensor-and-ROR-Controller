#ifndef WAITMILLISECONDS
#define WAITMILLISECONDS 
#include "ArduinoTimer.h"          // https://www.megunolink.com/documentation/arduino-libraries/arduino-timer/
ArduinoTimer Timer; 
//The methods supported by the ArduinoTimer are:
//
//Reset(); Resets the timer to the current value of the millis timer
//EllapsedMilliseconds(); Returns the number of milliseconds that have passed since the timer was last reset
//EllapsedSeconds(); Returns the number of seconds that have passed since the timer was last reset
//TimePassed_Milliseconds(Period, AutoReset — optional); Returns true if Period milliseconds have elapsed since the timer was last reset. If AutoReset is true, or not present, the timer will be reset if the period has passed
//TimePassed_Seconds(Period, AutoReset — optional); Returns true if Period seconds have elapsed since the timer was last reset. If AutoReset is true, or not present, the timer will be reset if the period has passed
//TimePassed_Minutes(Period, AutoReset — optional); Returns true if Period minutes have elapsed since the timer was last reset. If AutoReset is true, or not present, the timer will be reset if the period has passed
//TimePassed_Hours(Period, AutoReset — optional); Returns true if Period hours have elapsed since the timer was last reset. If AutoReset is true, or not present, the timer will be reset if the period has passed

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                           void waitMilliseconds(uint32_t Milliseconds){
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Timer.Reset();while(!Timer.TimePassed_Milliseconds(Milliseconds));                         // non-blocking delay delay(100);
}
#endif
