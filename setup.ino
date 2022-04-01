
// see https://github.com/espressif/arduino-esp32/commit/f6c9faf4daeff4ac6bcfafd55dbb1fb922b304ed




// SETUP
//***********************************************************************************
                                       void setup() {
//************************************************************************************
  pixels.begin();
  M5.Lcd.begin();
  M5.begin();
  M5.Power.begin();
  Wire.begin();
  Wire.setClock(400000);                                                          // Increase I2C clock speed to 400kHz
  Serial.begin(9600);                                                             // 9600 needed for ROR

  waitMilliseconds(100);                                                          // non-blocking delay 

  Serial.setDebugOutput(true);
  
   /////////////////////  MLX90640 //////////////////////////
  MLX90640setupOK =  MLX90640setup();
  
  
  ////////////////// //  Wi-Fi ////////////////////////
  Serial.println("connect Wi-Fi");
  WiFiconnected = ConnectWiFi(false);                                             // Call connect and tell it you are not connected save result in global                             

  
/////////////////////  OTA //////////////////////////
    ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
  /////////////////////////////////////////////////////////



  


 /////////////////////  LCD //////////////////////////
 
 
 
  //Once params are extracted, we can release eeMLX90640 array

////////////////// Turn Off NEO Pixels om FIRE ///////////////////////
  pixels.begin();
  pixels.show();
  

///////////////////////// WebServer  /////////////////////////////

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){             // Route for root / web page
    request->send_P(200, "text/html", index_html, processor);
    //request->send_P(200, "text/html", index_html);
  });
  server.on("/percentcloud", HTTP_GET, [](AsyncWebServerRequest *request){  // Temperature 
    request->send_P(200, "text/plain", getpercentCloud().c_str());
  });
  server.on("/tempmax", HTTP_GET, [](AsyncWebServerRequest *request){      // Max Temperature
    request->send_P(200, "text/plain", getMaxTemp().c_str());
  });
  server.on("/tempmin", HTTP_GET, [](AsyncWebServerRequest *request){      // Min Temperature
    request->send_P(200, "text/plain", getMinTemp().c_str());
  });
  server.on("/roof", HTTP_GET, [](AsyncWebServerRequest *request){      // Min Temperature
    request->send_P(200, "text/plain", getRoof().c_str());
  });
  server.on("/thermal", HTTP_GET, [](AsyncWebServerRequest *request){       // Thermal.bmp
    request->send(SD, "/thermal.bmp", "image/bmp", false);
  });
 
  server.begin();                  //Start server
  Serial.println("HTTP server started");

}
         
