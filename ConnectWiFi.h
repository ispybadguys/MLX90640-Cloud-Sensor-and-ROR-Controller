#ifndef CONNECTWIFI
#define CONNECTWIFI
#include "ArduinoTimer.h"          // https://www.megunolink.com/documentation/arduino-libraries/arduino-timer/
ArduinoTimer WiFiTimer;
bool  WiFiconnected = false;                                                  // WiFi Isn't Connected
#define CHECK_WIFI_EVERY 5                                                    // Time in minutes for rechecking connection 
//The methods supported by the ArduinoTimer are:
//
//Reset(); Resets the timer to the current value of the millis timer
//EllapsedMilliseconds(); Returns the number of milliseconds that have passed since the timer was last reset
//EllapsedSeconds(); Returns the number of seconds that have passed since the timer was last reset
//TimePassed_Milliseconds(Period, AutoReset — optional); Returns true if Period milliseconds have elapsed since the timer was last reset. If AutoReset is true, or not present, the timer will be reset if the period has passed
//TimePassed_Seconds(Period, AutoReset — optional); Returns true if Period seconds have elapsed since the timer was last reset. If AutoReset is true, or not present, the timer will be reset if the period has passed
//TimePassed_Minutes(Period, AutoReset — optional); Returns true if Period minutes have elapsed since the timer was last reset. If AutoReset is true, or not present, the timer will be reset if the period has passed
//TimePassed_Hours(Period, AutoReset — optional); Returns true if Period hours have elapsed since the timer was last reset. If AutoReset is true, or not present, the timer will be reset if the period has passed
//StartTime(); The value of the millis counter when the timer was last reset 

IPAddress ip(192, 168, 1, 105);
IPAddress dns(8, 8, 8, 8);                       
IPAddress gateway(192, 168, 1, 1);                                                // set it to the same address as ip
IPAddress subnet(255, 255, 255, 0);
String hostName = "SKYmonitor";                                                   // define a hostName

/////////////////////////////////////////////////////////////////////////////
bool TestConnectedWiFi(void) {                                                    // returns connection state
/////////////////////////////////////////////////////////////////////////////
//////////////////// Test Connected Messsage LCD ////////////////////////////
                                                              
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {                              
     LCDMessage("Wi-Fi Not Connected");                                           // Message on LCD
     //Serial.println("TestConnectedWiFi: Wi-Fi Not Connected");                  // print not connected
     return(false);
     // ESP.restart(); 
  }
  return(true);                                                                     // WiFi Is Connected exit quietly
}
/////////////////////////////////////////////////////////////////////////////
bool ConnectWiFi(bool quiet) {                                                      // connect wifi and return true if connected
/////////////////////////////////////////////////////////////////////////////
  WiFiTimer.Reset();                                                                // reset the timmer                         
  WiFi.mode(WIFI_STA);                                                              // Station mode
//  WiFi.config(ip, dns, gateway, subnet);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);                  // DHCP to assign IP address
  
  if(!WiFi.hostname(hostName.c_str())) Serial.println("WiFi.hostname Hostname failed");
   waitMilliseconds(3000);                                                           // non-blocking delay  
  WiFi.begin(SECRET_SSID, SECRET_PSW);

  waitMilliseconds(3000);                                                            // non-blocking delay  

  bool isConnected = TestConnectedWiFi();                                            // TEST CONNECTED now that you have tried
  if(!quiet) { 
  Serial.println("IP address: ");
  
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  Serial.printf("%s @",hostName.c_str());
  Serial.println(WiFi.localIP());
}
  if(isConnected) {
    LCDMessage(" RSSI: " + String(WiFi.RSSI()));
    LCDMessage(hostName + "@" +  WiFi.localIP().toString());
  } else {
    LCDMessage("Wi-Fi Connection Failed");}
  }

/////////////////////////////////////////////////////////////////////////////
void testWiFiConnection(void) {                                                     // if was connected test every CHECK_WIFI_EVERY minutes
/////////////////////////////////////////////////////////////////////////////
if(!WiFiTimer.TimePassed_Minutes(CHECK_WIFI_EVERY)) return;                         // if connected test every 5 minutes
if(TestConnectedWiFi() == false) ConnectWiFi(true);                                 // connect Wifi and return state no status printing
}
#endif
