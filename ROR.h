// Roll-off Roof Code original by Chuck Faranda https://ccdastro.com/RRCI.zip
// https://ccdastro.com/RRCI.zip Includes"RCCI Setup.exe" the ASCOM Driver
// Modified by Kurt R Lanes March 2022 to run on M5Stack FIRE or Core2 and use Ultrasound roof position
// Connect  Grove Hub to FIRE Port B
// Connect M5Stack Relay to Grove Hub https://shop.m5stack.com/products/mini-3a-relay-unit?_pos=2&_sid=3016b2ff8&_ss=r
// Connect Ultrasonic Distance Unit I/O (RCWL-9620) to Grove Hub https://shop.m5stack.com/products/ultrasonic-distance-unit-i-o-rcwl-9620?_pos=2&_sid=bbb39dc1e&_ss=r&variant=42084187930881
// For Cloud Sensor connect Thermal Camera Unit (MLX90640) to Port A https://shop.m5stack.com/products/thermal-camera?_pos=3&_sid=f924f6e91&_ss=r&variant=16804741939290
// Press Button A for > 200 ms sends roof stop
// Press Button B for > 200 ms sends roof open
// Press Button C for > 200 ms sends roof close
// Press Button A for > 1000 ms sends stop and powers off
#include <Adafruit_NeoPixel.h>
#include "UNIT_4RELAY.h"
UNIT_4RELAY unit_4relay;                                                                // 4_RELAY Relay 0 is Open Command Relay 1 is Cycle Command
#include "M5_ADS1100.h"
ADS1100 ads;                                                                            // Used as simple 5V input to sense Telescope Safe Below Roof Line

void   open_roof_command(void);                                                         // Prototype Checks if closed and if so outputs 500ms pulse on the CYCLE command line else <no op>
void   close_roof_command(void);                                                        // Prototype Checks if open and if so outputs 500ms pulse on the CYCLE command line else <no op>
void   stop_roof_command(void);                                                         // Prototype Checks if moving and if so outputs 500ms pulse on the CYCLE command line
void   LCDroofState(float Distance,float RoofSpeed,String state,String safe,bool isGet);// Prototype display Message in roof status line
String roof_state(bool isGET);                                                          // Prototype roof_state: get position and speed
void   ROR();                                                                           // Prototype Roll off roof
void   LCDMessage(String Message);                                                      // Prototype clear LCD and display Message and delay 1 second
String check_telescope_safe(void);                                                      // Prototype

#define UltrasonicTrigPin     26                // Port B trigger
#define UltrasonicEchoPin     36                // Port B echo
#define RelayPin              26                // Port B trigger shares this pin but pulsing both
#define ASCOM_TIMEOUT         20                // No communication timeout initialize to timmed out
#define RoofOPEN             3048.              // distance in mm > RoofOPEN is open
#define RoofCLOSED            600.              // 600 < distance < RoofOPEN is closed
#define OPEN_PIN              16                // Low pulse opens roof
uint8_t OPEN_RELAY    =       0;                // Low pulse opens roof Relay Number for i2c 4-Relay
uint8_t CYCLE_RELAY   =       1;                // Low pulse cycles roof Relay Number for i2c 4-Relay
#define CYCLE_PIN             17                // Low pulse cycles roof
#define ROOF_UNOBSTRUCTED_PIN 26                // High signals safe
#define SPARE_PIN             35                // connected but unused
#define M5STACK_FIRE_NEO_NUM_LEDS 10
#define M5STACK_FIRE_NEO_DATA_PIN 15

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);

String    serialin;                             // incoming ASCOM serial data
String    str;                                  // store the state of opened/closed/safe pins
float     Distance       = 0;                   // save distance from last call to roof_state
bool      roofOPEN       = false;               // State of Roof
int32_t   ASCOMconnected = ASCOM_TIMEOUT;       // check if communicating
String    MotorState = "not_moving_o#";
String    GETstring  = "closed,safe,not_moving_c#";
String    safe;
int       getCounter = 0; 
SONIC_IO sensor;                                // create Ultrasonic Sensor Object

////////////////////////////////////////////////////////////////////////////////  
                            void ROR() {
//////////////////////////////////////////////////////////////////////////////// 

/////////////////////  RELAY/Ultrasonic  INIT /////////////////////////////////

 sensor.begin(UltrasonicTrigPin,UltrasonicEchoPin);                // begin(uint8_t trig = 26, uint8_t echo = 36); PORT B
 pinMode(OPEN_PIN,OUTPUT);                                         // Set Open Command
 digitalWrite(OPEN_PIN,HIGH);                                      // Low Pulse Opens
 pinMode(CYCLE_PIN,OUTPUT);                                        // Set Open Command
 digitalWrite(CYCLE_PIN,HIGH);                                     // Low Pulse Cycles Roof
 pinMode(ROOF_UNOBSTRUCTED_PIN,INPUT);                             // Roof Obstructed = High OK to OPEN/CLOSE
 unit_4relay.switchMode(true);                                     // Init 4-RELAY
 unit_4relay.relayALL(false);                                      // Open all the relays.
 ads.getAddr_ADS1100(ADS1100_DEFAULT_ADDRESS);                     // 0x48, 1001 000 (ADDR = GND) he address can be changed
 ads.setGain(GAIN_ONE);                                            // 1x gain(default)
 ads.setMode(MODE_SINGLE);                                         // Single-conversion mode Continuous conversion mode (default)
 ads.setRate(RATE_8);                                              // 8SPS (default)
 
 /////////////////////  Check Buttons Open/Close //////////////////////////////
 
   GETstring = roof_state(false);                                 // get the roof state and don't update the LCD
   safe = check_telescope_safe();                                 // get safe condition   
   M5.update();                                                   // Check Power Off. 
    if (M5.BtnA.pressedFor(200)) {
       stop_roof_command();
       LCDMessage("Button Stop Roof");                                                        // clear LCD and display Message and delay 1 second
      }
      
    if (M5.BtnB.pressedFor(200)) {
       LCDMessage("Button Open Roof");                                                        // clear LCD and display Message and delay 1 second
       open_roof_command();}

      if (M5.BtnC.pressedFor(200)) {
         LCDMessage("Button Close Roof");                                                      // clear LCD and display Message and delay 1 second
         close_roof_command();}
      
  
  ////////////////////////  Check ASCOM_TIMEOUT ///////////////////////////////// 
     
  if (Serial.available()==0 && ASCOMconnected>ASCOM_TIMEOUT) Serial.write("RRCI#");             // Output init string unit if ASCOM_TIMEOUT
  ASCOMconnected++;                                                                             // bump timer ASCOM_TIMEOUT

  ////////////////////////  Check ASCOM Command /////////////////////////////////    
  while (Serial.available()>0) { //Read Serial data and allocate on serialin
    ASCOMconnected = 0;                                                                          // stop init messages by reseting ASCOM_TIMEOUT
    
    serialin = Serial.readStringUntil('#');                                                      // Read command fro ASCOM Driver

      if      (serialin == "x")     {stop_roof_command();LCDMessage("ASCOM Abort Open Roof");}   // stop opening: Checks if moving and if so outputs 500ms pulse on the CYCLE command line
      else if (serialin == "y")     {stop_roof_command();LCDMessage("ASCOM Abort Close Roof");}  // stop opening: Checks if moving and if so outputs 500ms pulse on the CYCLE command line
      else if (serialin == "open")  {open_roof_command();LCDMessage("ASCOM Open Roof");}         // Open Command: Checks if closed and if so outputs 500ms pulse on the CYCLE command line else <no op>
      else if (serialin == "close") {close_roof_command();LCDMessage("ASCOM Close Roof");}       // Close Command:Checks if open and if so outputs 500ms pulse on the CYCLE command line else <no op>
      else if (serialin == "restart")ESP.restart();                                              // Restart the ESP32
      else if (serialin == "force")ESP.restart();                                              // Restart the ESP32
      else if (serialin == "get")   {Serial.print(GETstring);                                    // get Command
         GETstring = roof_state(true);
         serialin = "";                                                                          // clear command string
         str = "";                                                                               // clear response string
         waitMilliseconds(100);                                                                  // non-blocking delay  wait a bit for another command
      }
    }  // end while (Serial.available()>0)
 
}  // end ROR
////////////////////////////////////////////////////////////////////////////////  
                           String check_telescope_safe(void){
////////////////////////////////////////////////////////////////////////////////  
// check_telescope_safe Checks if the telescope is safe to open or close 
  String safe;                                                                                     // String for check obstructed
  byte error;
  int8_t address;
  ads.setOSMode(OSMODE_SINGLE);                                                                     // Set to start a single-conversion. 
  safe = "unsafe";                                                                                  // default is unsafe                                                                
  address = ads.ads_i2cAddress;
  Wire.beginTransmission(address);
  error = Wire.endTransmission();
  
  if (error != 0) return("unsafe");                                                                  // If the device is not connected.
 
    int16_t result;
    result = ads.Measure_Differential();
    // printf("ADC =%d\n", result);
    if(result > 10000) return("safe");                                                               // More that 4 volts then safe reads 11227
    else return("unsafe"); 

//    if(digitalRead(ROOF_UNOBSTRUCTED_PIN)!=0) safe = "safe";                                      // Roof Obstructed = Low is unsafe to OPEN/CLOSE
//    else safe = "unsafe";                                                                         // Roof Obstructed = Low is unsafe to OPEN/CLOSE          
  
}  
  
////////////////////////////////////////////////////////////////////////////////  
                           void open_roof_command(void){
////////////////////////////////////////////////////////////////////////////////  
// Open Command: Checks if closed and if so outputs 100ms pulse on the OPEN command line else <no op>
// If roof closed but obstructed return status and take no action
   String safe    = check_telescope_safe();                                                               // check obstructed
  if (safe != "safe") {                                                                                   // DON'T Touch anything
    LCDMessage("ROOF IS OBSTRUCTED");
    GETstring ="closed,"  + safe + "," + "not_moving_o#";                                                // The next get command will return this
    return;}  
    if (roofOPEN) {
    LCDMessage("Roof is Open");                                                                          // if roof is open send open command anyway
        GETstring ="opened,"  + safe + "," + "not_moving_o#";                                            // The next get command will return this
        digitalWrite(OPEN_PIN,LOW);                                                                      // Low Pulse = OPEN Roof might be partially open
        unit_4relay.relayWrite(OPEN_RELAY,true);                                                         // Close the relay OPEN_RELAY
        waitMilliseconds(100);                                                                           // non-blocking delay  
        digitalWrite(OPEN_PIN,HIGH);                                                                     // Low Pulse OPENS
        unit_4relay.relayWrite(OPEN_RELAY,false);                                                        // Open the relay OPEN_RELAY
        return;
     } else {                                                                                            // Roof is closed send open cycle for non-4 relay
        digitalWrite(OPEN_PIN,LOW);                                                                      // Low Pulse = OPEN Roof might be partially open
        digitalWrite(RelayPin, LOW);                                                                     // Cycle relay if not using 4-relay
        unit_4relay.relayWrite(OPEN_RELAY,true);                                                         // Close the relay OPEN_RELAY
        waitMilliseconds(100);                                                                           // non-blocking delay  
        digitalWrite(OPEN_PIN,HIGH);                                                                     // Low Pulse OPENS
        unit_4relay.relayWrite(OPEN_RELAY,false);                                                        // Open the relay OPEN_RELAY         
        digitalWrite(RelayPin, LOW);
        GETstring ="unknown,"  + safe + "," + "moving,";                                                 // The next get command will return roof moving;
        LCDMessage("Open Pulse Sent");
     }   // END ELSE roof is closed
}
//////////////////////////////////////////////////////////////////////////////// 
                           void cycle_relay(void){
////////////////////////////////////////////////////////////////////////////////
  digitalWrite(RelayPin, HIGH);                                                                      // Pulse the cycle pin to close the roof
  digitalWrite(CYCLE_PIN,LOW);                                                                       // Low Pulse = Cycle
  unit_4relay.relayWrite(CYCLE_RELAY,true);                                                          // Close the relay CYCLE_RELAY
  waitMilliseconds(100);                                                                             // non-blocking delay  
  unit_4relay.relayWrite(CYCLE_RELAY,false);                                                         // Open the relay CYCLE_RELAY
  digitalWrite(RelayPin, LOW);                                                                       // Open single relay is used
  digitalWrite(CYCLE_PIN,HIGH);                                                                      // Low Pulse Cycles set high
}  
//////////////////////////////////////////////////////////////////////////////// 
                           void close_roof_command(void){
////////////////////////////////////////////////////////////////////////////////  
// Checks if ULTRASOUND shows opwn and if so outputs 100ms pulse on the CYCLE command line else <no op>
// If roof open but obstructed return status and take no action

  String safe    = check_telescope_safe();                                                           // check obstructed 
  if (!roofOPEN) {
    LCDMessage("NO-OP Roof is Closed");
    GETstring ="closed,"  + safe + "," + "not_moving_c#";                                            // The next get command will return this
    return;}
  if (safe != "safe") {                                                                              // DON'T Touch anything
    LCDMessage("ROOF IS OBSTRUCTED");
    GETstring ="open,"  + safe + "," + "not_moving_o#";                                              // The next get command will return this
    return;}  
  cycle_relay();                                                                                     // pulse the cycle relay  
  GETstring ="unknown,"  + safe + "," + "moving,";                                                   // The next get command will return this;
  LCDMessage("CLOSE Pulse Sent");                                                                    // clear LCD and display Message and delay 1 second
 }
////////////////////////////////////////////////////////////////////////////////  
                           void stop_roof_command(void){
//////////////////////////////////////////////////////////////////////////////// 
// Checks if moving and if so outputs 100ms pulse on the CYCLE command line else <no op>

  float lastDistance  = Distance;                                                                    // save roof location from last open close or stop 
  String CurrentState = roof_state(false);
  String safe         = check_telescope_safe();                                                      // check obstructed
  
  if (abs(Distance - lastDistance) < 10.) {                                                          // if motion less than 2x the noise distance roof is moving
    LCDMessage("NO-OP Roof is Stopped");
    GETstring ="unknown,"  + safe + "," + "not_moving_o#";                                           // The next get command will return this
    return;}        
  digitalWrite(RelayPin, HIGH);                                                                      // Pulse the cycle pin
  digitalWrite(CYCLE_PIN,LOW);                                                                       // Low Pulse = Cycle
  unit_4relay.relayWrite(CYCLE_RELAY,true);                                                          // Close the relay CYCLE_RELAY
  waitMilliseconds(100);                                                                             // non-blocking delay  
  unit_4relay.relayWrite(CYCLE_RELAY,false);                                                         // Open the relay CYCLE_RELAY
  digitalWrite(CYCLE_PIN,HIGH);                                                                      // Low Pulse = Cycle
  digitalWrite(RelayPin, LOW);
  GETstring ="unknown,"  + safe + "," +   "not_moving_o#";                                           // The next get command will return this
  LCDMessage("STOP Pulse Sent");                                                                     // clear LCD and display Message and delay 1 second
} 


 ////////////////////////////////////////////////////////////////////////////////
                         String roof_state(bool isGet){
 ////////////////////////////////////////////////////////////////////////////////
 // roof_state: get position and speed and update LCD
  String state;
  uint32_t time = millis();                                                           // measure roof speed
  Distance = sensor.getDistance();                                                    // Serial.printf("Distance: %.2fmm\r\n",Distance);
  String safe    = check_telescope_safe();                                            // check obstructed
  roofOPEN       = (Distance > RoofOPEN);                                             // State of Roof
 
  waitMilliseconds(100);                                                              // non-blocking delay                                                                         

   float RoofSpeed = (Distance - sensor.getDistance())/(millis()-time);               // Serial.printf("Distance: %.2fmm\r\n",Distance);
   if      (roofOPEN)             state= "opened,"  + safe + "," + "not_moving_o#";   // Roof is open  and not moving
   else                           state= "closed,"  + safe + "," + "not_moving_c#,";  // Roof is closed and not moving
   if      (abs(RoofSpeed)> 0.1 ) state= "unknown," + safe + "," + "moving#,";        // Roof is moving
   LCDroofState(Distance,RoofSpeed,state,safe,isGet);                                 // update status line get command
   return(state);                        
   } 
////////////////////////////////////////////////////////////////////////////////
                         void LCDroofState(float Distance,float RoofSpeed,String state,String safe,bool isGet){
/////////////////////////////////////////////////////////////////////////////////
   int color;
   M5.Lcd.setTextSize(1);
   M5.Lcd.setCursor(36, 199);                                                          // Roof text area
   if (safe == "safe") color = TFT_BLUE;                                               // color Area just below image for Roof Distance text area.
     else color = TFT_RED;
   M5.Lcd.fillRect(0, 199, 320, 8, color);                                             // Clear Area just below image for Roof Distance text area.
     
   M5.Lcd.setCursor(0, 199);                                                           // Roof text area was 36  
   M5.Lcd.printf("%d %4.0fmm %5.1fm/s ",getCounter++,Distance,RoofSpeed);              // write the disance speed in m/s
   if (isGet) {
     M5.Lcd.setTextColor(YELLOW, color);
     M5.Lcd.print(GETstring);                                                          // write the disance speed in m/s
   }

////// NEO Pixels for Roof Position

     float threshold = 0;
     uint8_t  red, green, blue;
     ////// display red bar length of percent closed if unsafe else blue bar
     if (safe == "safe") {red = 0; green =0; blue = 30;} else {red = 30; green =0; blue = 0;}
     for (uint8_t n = 0; n < M5STACK_FIRE_NEO_NUM_LEDS/2; n++){
        pixels.setPixelColor(n, pixels.Color(0, 30, 0));                                // left bar
        pixels.setPixelColor(n+5, pixels.Color(0, 30, 0));                              // right bar
        if (Distance > threshold) {
          pixels.setPixelColor(n, red, green, blue);                                    // left bar
          pixels.setPixelColor(n+5, red, green, blue);                                  // left bar
        }
        threshold += RoofOPEN/(M5STACK_FIRE_NEO_NUM_LEDS/2);                            // divide into the 5 bars
  }
  pixels.show();
}
 // LCDMessage: get position and speed and update LCD   
 ////////////////////////////////////////////////////////////////////////////////
                         void LCDMessage(String Message){
 ////////////////////////////////////////////////////////////////////////////////
 // LCDMessage: get position and speed and update LCD
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.drawCentreString(Message, 160, 80, 4);  
    waitMilliseconds(3000);                                                             // non-Blocking Wait
    return;
 }
 
