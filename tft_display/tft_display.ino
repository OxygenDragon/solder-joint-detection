#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <SPI.h>
#include <Wire.h>
#include <string.h>

#define TFT_CS   9 // TFT LCD CS PIN
#define TFT_DC   7 // TFT DC(A0、RS) 
#define TFT_RST  8 // TFT Reset


Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
int has_defect = 0;
char received_str[4][5];
char display_str[4][20] = {0};
unsigned long last;

void setup(void) {
  
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab

  tft.setRotation(1);
  tft.setCursor(5, 30);
  tft.setTextColor(ST77XX_WHITE);
  tft.setFont(&FreeSerif12pt7b);
  tft.print("Joint Detection");
  tft.setCursor(5, 60);
  tft.setTextColor(ST77XX_RED);
  tft.setFont(&FreeSansBold9pt7b);
  tft.print("I2C Not Working");
  tft.setCursor(0, 80);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setFont(&FreeSans9pt7b);

  Wire.begin(0x12);
  Wire.onReceive(receiveEvent);
  Serial.begin(57600);
  Serial.println("I2C Slave Started");
  tft.fillScreen(0);

  last = millis();
}

void receiveEvent(int numBytes){
  
  Serial.println("received");                                                                                                                                         
  tft.fillScreen(0);
  int str_index = 0;
  int char_index = 0;
  
  // get i2c data 
  while(Wire.available()){
    char c = Wire.read();
    if (c == ',') {
      received_str[str_index++][char_index] = '\0';
      char_index = 0;
    } else {
      received_str[str_index][char_index++] = c;
    }
  }

  has_defect = (received_str[0][0] == '1');

  // decode received data to display strings
  if (has_defect) {
    sprintf(display_str[0], "Defect detected!");
  } else {
    sprintf(display_str[0], "QC PASSED!!!");
  }
  sprintf(display_str[1], "Insufficient: %s", received_str[1]);
  sprintf(display_str[2], "Short: %s", received_str[2]);
  sprintf(display_str[3], "Too much: %s", received_str[3]);

  Serial.println(display_str[0]);
}

void loop() {
  unsigned long now = millis();
  if (now - last < 50) return;
  if (!has_defect) {
    tft.setCursor(20, 50);
    tft.setTextSize(1.8);
    tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
    tft.print(display_str[0]);
  } else {
    tft.setCursor(15, 23);
    tft.setTextSize(1.8);
    tft.setTextColor(ST7735_BLUE, ST7735_BLACK);
    tft.print(display_str[0]);
    
    for (int i = 1; i < 4; ++i) {
      tft.setCursor(15, 23 + 25 * (i));
      tft.setTextSize(1);
      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
      tft.print(display_str[i]);
    } 
  }
  last = now;
}
