#include <SD.h>
#include <sd_defines.h>
#include <sd_diskio.h>
#include "Arduino_secrets.h"                    // Place to keep your SSID and Secret keys

void ThermalImageToWeb(float mlx90640To[], float MinTemp, float MaxTemp);

//--------------------------------------------------------------------------------------------------------//
// Thermal buffer Camera - Project using ESP8266 or ESP32 and MLX90640 sensor (32x24 px)                  //
// Author: Szymon Baczyński                                                                               //
// Date: April 2019                                                                                       //
// Version: Ver 1.1                                                                                       //
// GitHub: https://github.com/Samox1/ESP_Thermal_Camera_WebServer                                         //
// Modified to Use M5STACK LCD and to Use as a Cloud Cover Sensor using elements from                     //
// M5STACK MLX90640 Demo Code and Szymon Baczyński's development                                          //                                                                            //               
// Author: Kurt R Lanes                                                                                   // 
// Date: March 2022                                                                                       //
// Using M5STACK FIRE should work with CORE2 Also                                                         //
// Connect MLX60450 to Port A                                                                             //
// Connect Mini 3A Relay Unit to Port B                                                                   //
// Connect Ultrasonic Distance Unit (RCWL-9600) to Port B usin 1 to 3 HUB Expansion Unit                  //
// Connect USB to Computer and load ASCOM driver RRCI Driver                                              //
// Install FAT Formatted SD card in M5STACK                                                               //
// Image View on IP Address port 80 Tested with Chrome and Safri. Firefox updates only on page refresh    //
// ROR control uses connected to CYCLE input on GAte Opener Configure ASCOM as 2-button control           //
//--------------------------------------------------------------------------------------------------------//

#include <UNIT_SONIC.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <M5Stack.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_SSD1331.h>
#include "MLX90640_I2C_Driver.h"
#include "MLX90640_API.h"

//StartTime(); The value of the millis counter when the timer was last reset 
#include "waitMilliseconds.h"
#include "ROR.h"                  // Roll-off-roof code include locally
#include "ConnectWiFi.h"          // Wi-Fi connect code included locally

////////////// Function Prototypes ///////////////////////////    
void displayImageOnLCD(float mlx90640To[], float MinTemp, float MaxTemp);             // Function Prototype

bool RanOnce = false;
bool MLX90640setupOK = false;

const byte MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640

#define TA_SHIFT 8 //Default shift for MLX90640 in open air

float mlx90640To[768];
paramsMLX90640 mlx90640;

// You can use any (4 or) 5 pins
const int sda    = 21;         // Ultrasonic Port A trigger
const int scl    = 22;         // Ultrasonic Port A echo
const int   g26  = 26;         // Ultrasonic Port B trigger or SDA
const int   g36  = 36;         // Ultrasonic Port B echo or SCL
const int txd2   = 17;         // Ultrasonic Port C trigger or SDA
const int rxd2   = 16;         // Ultrasonic Port C echo or SCL
#define sclk 18
#define mosi 23
#define cs   17
#define rst  5
#define dc   16

float thresholdCloud = -20;                     // Temperature of clear sky degrees C
float percentCloud  = 100;                      // Percent Cloud Cover clear sky < thresholdCloud
int   cloudPixels   = 0;

// Color definitions
#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

#define PART_BOUNDARY "123456789000000000000987654321"

//the colors we will be using
const uint16_t camColors[] = {0x480F,
                              0x400F, 0x400F, 0x400F, 0x4010, 0x3810, 0x3810, 0x3810, 0x3810, 0x3010, 0x3010,
                              0x3010, 0x2810, 0x2810, 0x2810, 0x2810, 0x2010, 0x2010, 0x2010, 0x1810, 0x1810,
                              0x1811, 0x1811, 0x1011, 0x1011, 0x1011, 0x0811, 0x0811, 0x0811, 0x0011, 0x0011,
                              0x0011, 0x0011, 0x0011, 0x0031, 0x0031, 0x0051, 0x0072, 0x0072, 0x0092, 0x00B2,
                              0x00B2, 0x00D2, 0x00F2, 0x00F2, 0x0112, 0x0132, 0x0152, 0x0152, 0x0172, 0x0192,
                              0x0192, 0x01B2, 0x01D2, 0x01F3, 0x01F3, 0x0213, 0x0233, 0x0253, 0x0253, 0x0273,
                              0x0293, 0x02B3, 0x02D3, 0x02D3, 0x02F3, 0x0313, 0x0333, 0x0333, 0x0353, 0x0373,
                              0x0394, 0x03B4, 0x03D4, 0x03D4, 0x03F4, 0x0414, 0x0434, 0x0454, 0x0474, 0x0474,
                              0x0494, 0x04B4, 0x04D4, 0x04F4, 0x0514, 0x0534, 0x0534, 0x0554, 0x0554, 0x0574,
                              0x0574, 0x0573, 0x0573, 0x0573, 0x0572, 0x0572, 0x0572, 0x0571, 0x0591, 0x0591,
                              0x0590, 0x0590, 0x058F, 0x058F, 0x058F, 0x058E, 0x05AE, 0x05AE, 0x05AD, 0x05AD,
                              0x05AD, 0x05AC, 0x05AC, 0x05AB, 0x05CB, 0x05CB, 0x05CA, 0x05CA, 0x05CA, 0x05C9,
                              0x05C9, 0x05C8, 0x05E8, 0x05E8, 0x05E7, 0x05E7, 0x05E6, 0x05E6, 0x05E6, 0x05E5,
                              0x05E5, 0x0604, 0x0604, 0x0604, 0x0603, 0x0603, 0x0602, 0x0602, 0x0601, 0x0621,
                              0x0621, 0x0620, 0x0620, 0x0620, 0x0620, 0x0E20, 0x0E20, 0x0E40, 0x1640, 0x1640,
                              0x1E40, 0x1E40, 0x2640, 0x2640, 0x2E40, 0x2E60, 0x3660, 0x3660, 0x3E60, 0x3E60,
                              0x3E60, 0x4660, 0x4660, 0x4E60, 0x4E80, 0x5680, 0x5680, 0x5E80, 0x5E80, 0x6680,
                              0x6680, 0x6E80, 0x6EA0, 0x76A0, 0x76A0, 0x7EA0, 0x7EA0, 0x86A0, 0x86A0, 0x8EA0,
                              0x8EC0, 0x96C0, 0x96C0, 0x9EC0, 0x9EC0, 0xA6C0, 0xAEC0, 0xAEC0, 0xB6E0, 0xB6E0,
                              0xBEE0, 0xBEE0, 0xC6E0, 0xC6E0, 0xCEE0, 0xCEE0, 0xD6E0, 0xD700, 0xDF00, 0xDEE0,
                              0xDEC0, 0xDEA0, 0xDE80, 0xDE80, 0xE660, 0xE640, 0xE620, 0xE600, 0xE5E0, 0xE5C0,
                              0xE5A0, 0xE580, 0xE560, 0xE540, 0xE520, 0xE500, 0xE4E0, 0xE4C0, 0xE4A0, 0xE480,
                              0xE460, 0xEC40, 0xEC20, 0xEC00, 0xEBE0, 0xEBC0, 0xEBA0, 0xEB80, 0xEB60, 0xEB40,
                              0xEB20, 0xEB00, 0xEAE0, 0xEAC0, 0xEAA0, 0xEA80, 0xEA60, 0xEA40, 0xF220, 0xF200,
                              0xF1E0, 0xF1C0, 0xF1A0, 0xF180, 0xF160, 0xF140, 0xF100, 0xF0E0, 0xF0C0, 0xF0A0,
                              0xF080, 0xF060, 0xF040, 0xF020, 0xF800,
                             };

//WebServer server(80);
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

float p = 3.1415926;


float MaxTemp = 0;
float MinTemp = 0;

/**********************************************************************************/
String getpercentCloud(){
/**********************************************************************************/
  extern float percentCloud;
  return String(percentCloud,0);
}
/**********************************************************************************/
String getMaxTemp(){
  /**********************************************************************************/
  extern float MaxTemp;
  return (String(MaxTemp,1) + "/" + String(MinTemp,1));
}
/**********************************************************************************/
String getMinTemp(){
  /**********************************************************************************/
  extern float  MinTemp;
  extern float  Distance;                     // Set in ROR
 
 // roof_state(false);                        // read roof distance
  return (String(Distance,1)+"/"+safe);
}
/**********************************************************************************/
String getRoof(){
  /**********************************************************************************/
  extern float Distance;
//  roof_state(false);                        // read roof distance
  return String(Distance,1);
}


/**********************************************************************************/
String processor(const String& var){  // Replaces placeholder with values
/**********************************************************************************/
  //Serial.println(var);
  if(var == "CLOUD"){
    return getpercentCloud();
  }
  if(var == "TEMPMAX"){
    return getMaxTemp();
  }
  if(var == "TEMPMIN"){
    return getMinTemp();
  }
  
  return String();
}


//Returns true if the MLX90640 is detected on the I2C bus
boolean isConnected()
{
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0)
    return (false); //Sensor did not ACK
  return (true);
}





void MLX_to_Serial(float mlx90640To[])
{
  for (int x = 0 ; x < 768 ; x++)
  {
    //Serial.print("Pixel ");
    Serial.print(x);
    Serial.print(": ");
    Serial.print(mlx90640To[x], 2);
    //Serial.print("C");
    Serial.println();
  }
}


int startTime;
//************************************************************************************
//************************************************************************************
//************************************************************************************
                                     void loop()  {
//************************************************************************************
    startTime = millis();

    testWiFiConnection();                                                      // test and reconnect every 5 minutes
      
    ArduinoOTA.handle();
 
    M5.update();                                                               // Check Power Off. 
    if (M5.BtnA.pressedFor(1000)) {
      M5.Lcd.fillScreen(TFT_BLACK);
      M5.Lcd.setTextColor(YELLOW, BLACK);
      M5.Lcd.drawCentreString("Power Off...", 160, 80, 4);
       waitMilliseconds(1000);                                                  // non-blocking delay 
      M5.powerOFF();
    }

 
    
  // Read Thermal buffer from MLX90640
  for (byte x = 0 ; x < 2 ; x++) //Read both subpages
  {
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
    if (status < 0)
    {
      Serial.print("GetFrame Error: ");
      Serial.println(status);
    }

    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

    float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
    float emissivity = 0.95;

    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
  }

    MaxTemp = -100;            // Initialize to find Max and Min Temperature
    MinTemp = +1E6;            // Initialize to find Max and Min Temperature
    cloudPixels = 0;           // counter for percent cloud
    
    for (int x = 0 ; x < 768 ; x++)                                 // Find Maximum and Minimum Temperature
    {
      if (mlx90640To[x] > MaxTemp) MaxTemp = mlx90640To[x];
      if (mlx90640To[x] < MinTemp) MinTemp = mlx90640To[x];
      if(mlx90640To[x]  > thresholdCloud)cloudPixels++;
      //Serial.printf("mlx90640To[x]  %.1f cloudPixels %d percentCloud %f\n",mlx90640To[x] ,cloudPixels,percentCloud);
    }
    percentCloud = ((float)cloudPixels / 768.)*100.;               // turn count to percentage
    // Serial.printf("cloudPixels %d percentCloud %f\n",cloudPixels,percentCloud);
    
    ThermalImageToWeb(mlx90640To, MinTemp, MaxTemp); 
   
    displayImageOnLCD(mlx90640To, MinTemp, MaxTemp);               // Function to draw Thermal buffer on OLED 

    //MLX_to_Serial(mlx90640To);
    //display.fillScreen(BLACK);
    
    ROR();                                                         // check roof driver
   
    waitMilliseconds(100);                                         // non-blocking delay 
}
//************************************************************************************
//************************************************************************************
