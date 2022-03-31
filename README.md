# MLX90640-Cloud-Sensor-and-ROR-Controller
This is M5STACK ESP32 Project that Displays a Thermal Image of the Sky on Webpage and on the local LCD. It also Calculates Cloud Cover and Controls a Roll-off Roof Observatory based on commands over USB from a ASCOM Driver.
The design is Based on the "Thermal buffer Camera" Project                  
Original code from: : Szymon Baczyński Dated: April 2019  GitHub: https://github.com/Samox1/ESP_Thermal_Camera_WebServer                                         
Modified by Kurt Lanes to Use M5STACK LCD and to Use as a Cloud Cover Sensor using elements from M5STACK MLX90640 Demo Code and Szymon Baczyński's 
development Dated: March 2022. The SPIFFS code was removed over concern for wearing out the SPIFFS memory with constant writes                                                                                      
This version is using M5STACK FIRE should work with CORE2 and can be modified to use any ESP32. If you use the I2C Ultrasonic Unit you can use a M5STACK CORE 

The code uses the Ultrasonic Unit facing the roof to determine the open/closed state without adding limit switches. The MLX90640 images the sky and
counts cold pixels to determine % cloud cover

Parts Needed :
MLX90640 I used the M5STACK Version
M5STACK 4-Relay Unit Relay 0 is wired between ground and the Open Command Line Relay 1 is connected between ground and the Cycle Command Line
The 4-Relay unit does not reset on power cycle or M5STACK reboot avoiding an unexpected roof open or close.
M5STACK ADS 1100 ADC is connected - to ground and + to the Safe Input in parallel with the optical obstruction sensor supplied. 
Since the Aleko uses 5 volt logic the ADC was used as a simple way to interface to the M5STACK without other components on a protoboard
M5STACK 1 to 3 Expansion Hub

Connect MLX90640 to Port A using 1 to 3 HUB Expansion Unit                                                                             
Connect M5STACK 4-Relay Unit to I2C using 1 to 3 HUB Expansion Unit 
Connect M5STACK ADS 1100 Unit to I2C using 1 to 3 HUB Expansion Unit 
Connect Ultrasonic Distance Unit (RCWL-9600) to Port B         
Connect USB to you Windows Computer and load ASCOM driver RRCI Driver. This seems to work fine with NINA                                             
Install FAT Formatted SD card (not ExFAT) in M5STACK SD Slot                                                             
The Thermal Image is Viewed on device IP Address port 80 Tested with Chrome and Safri. Firefox updates only on page refresh.
ROR control is designed for a Aleko style gate motor uses connected to CYCLE input and the Open input on Gate Opener 
Configure the ASCOM driver as 2-button control with Safety 
Create a file named Arduino_secrets.h with the following lines
#define SECRET_SSID "YOUR Wi-Fi Network"
#define SECRET_PSW "YOUR WiFi Password"

The page looks like this: 

<img width="1444" alt="image" src="https://user-images.githubusercontent.com/7380040/161166743-ead76ca1-d899-4d80-856b-61194177a1b5.png">

If I had any talent in CSS I could clean it up
The LCD also displays the image

As with any code development I cannot guarantee it will work correctly.  USE this at your own risk. If your scope can be damaged if it is not stowed duing 
roof operation you should not trust that this will provided the needed protection. The opener also poses a hazard for people and pets. The opener I used 
ignores the obstruction sensor during roof opens and only reverses on close operations. This code attempts to block any command if the Obstruction Signal 
is low. 
