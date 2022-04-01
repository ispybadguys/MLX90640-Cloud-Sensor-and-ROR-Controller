#define COLS 32
#define ROWS 24

#define INTERPOLATED_COLS 32               // It seems that larger interpolated image fail
#define INTERPOLATED_ROWS 24

void drawpixels(float *p, uint8_t rows, uint8_t cols, uint8_t boxWidth, uint8_t boxHeight, boolean showVal) ;
void interpolate_image(float *src, uint8_t src_rows, uint8_t src_cols, 
                       float *dest, uint8_t dest_rows, uint8_t dest_cols);
/*************************************************************************************/
       void displayImageOnLCD(float mlx90640To[], float MinTemp, float MaxTemp){
/*************************************************************************************/

  float dest_2d[INTERPOLATED_ROWS * INTERPOLATED_COLS];                       // Interpolated Array
  int ROWS_i,COLS_j;
  
  interpolate_image(mlx90640To, ROWS, COLS, dest_2d, INTERPOLATED_ROWS, INTERPOLATED_COLS);

  uint16_t boxWidth  = M5.Lcd.width() / INTERPOLATED_COLS;
  uint16_t boxHeight = (M5.Lcd.height()-31) / INTERPOLATED_ROWS; // 31 for bottom info
  drawpixels(dest_2d, INTERPOLATED_ROWS, INTERPOLATED_COLS, boxWidth, boxHeight, false);

  M5.Lcd.setTextSize(2);
//  M5.Lcd.fillRect(164, 220, 75, 18, TFT_BLACK);  // clear max temp text.  
//  M5.Lcd.fillRect(60, 220, 200, 18, TFT_BLACK); // clear spot temp text.  

/****************************************/
// Draw color bar
/****************************************/ 
  int icolor = 0;
  for (int icol = 0; icol <= 248;  icol++)
  {
    M5.Lcd.drawRect(36, 208, icol, 284 , camColors[icolor]);
    icolor++;
  }

 //  annotate colorbar at bottom
  M5.Lcd.setTextColor(TFT_WHITE);

  M5.Lcd.setCursor(1, 1);                                                      // update percent Cloud
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.printf("    Cloud Cover %5.1f%%",percentCloud);
  M5.Lcd.setCursor(45, 222);                                                   // update min & max temp. 

  M5.Lcd.printf("Min:%2.0fC",MinTemp,0);
  M5.Lcd.printf("     Max:%2.0fC",MaxTemp,0);

  int endTime = millis();
  float fps = 1000 / (endTime - startTime);                 //Serial.print("fps ");Serial.print(fps);Serial.println("");
  M5.Lcd.fillRect(300, 209, 20, 30, TFT_BLACK);             //Clear fps text area.  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(286, 210);
  M5.Lcd.printf("fps:");
  M5.Lcd.setCursor(288, 222);
  M5.Lcd.printf("%3.1f",fps);
  M5.Lcd.setTextSize(1);
}
float get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y) {
  if (x < 0)        x = 0;
  if (y < 0)        y = 0;
  if (x >= cols)    x = cols - 1;
  if (y >= rows)    y = rows - 1;
  return p[y * cols + x];
}/**************************************************************************/
void drawpixels(float *p, uint8_t rows, uint8_t cols, uint8_t boxWidth, uint8_t boxHeight, boolean showVal) {
/**************************************************************************/
int colorTemp;
  for (int y = 0; y < rows; y++) 
  {
    for (int x = 0; x < cols; x++) 
    {
      float val = get_point(p, rows, cols, x, y);
      
      if (val >= MaxTemp) 
        colorTemp = MaxTemp;
      else if (val <= MinTemp) 
        colorTemp = MinTemp;
      else colorTemp = val;

      uint8_t colorIndex = map(colorTemp, MinTemp, MaxTemp, 0, 255);
      colorIndex = constrain(colorIndex, 0, 255);// 0 ~ 255
      //draw the pixels!
      uint16_t color;
      color = val * 2;
      M5.Lcd.fillRect(boxWidth * x, boxHeight * y, boxWidth, boxHeight, camColors[colorIndex]);
    }
  }
}
