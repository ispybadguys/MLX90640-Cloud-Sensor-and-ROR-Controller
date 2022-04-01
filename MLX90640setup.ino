  bool MLX90640setup(void){
  Serial.println("MLX90640 Connecting");

  if (isConnected() == false)
  {
    Serial.println("MLX90640 not detected at default I2C address. WAITING 5 sec");
    LCDMessage("MLX90640 not detected");
    waitMilliseconds(5000);                                                          // non-blocking delay
    return(false); 
//  ESP.restart();
  } else {
    Serial.println("MLX90640 online!");
    LCDMessage("MLX90640 online!");
    
    //Get device parameters - We only have to do this once
    
    int status=0;
    uint16_t eeMLX90640[832];
    status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
    if (status != 0) {Serial.println("Failed to load system parameters");return(false);} 
    
    status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
    if (status != 0){Serial.println("Parameter extraction failed");return(false);}
  }
  int SetRefreshRate = 0;
  SetRefreshRate = MLX90640_SetRefreshRate(0x33,0x03);
  //int SetInterleavedMode = MLX90640_SetInterleavedMode(MLX90640_address);
  int SetChessMode = MLX90640_SetChessMode(MLX90640_address);
  return(true);
  }
