#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <SPI.h>
#include <Wire.h>

#define TFT_CS  10 // TFT LCD的CS PIN腳
#define TFT_DC   8 // TFT DC(A0、RS) 
#define TFT_RST  9 // TFT Reset


Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
char class_str[20];
char confidence_str[8];
int class_str_begin = 0;
int confidence_str_begin = 0;

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
  Serial.begin(9600);
  Serial.println("I2C Slave Started");
  tft.fillScreen(0);
}

void receiveEvent(int numBytes){
  int read_class = 1;
  class_str_begin = 0;
  confidence_str_begin = 0;
  
  while(Wire.available()){
    char c = Wire.read();
    Serial.print(c);
    if (read_class) {
      if (c == '\n') {
        read_class = 0;
        class_str[class_str_begin] = '\0';
        Serial.println();
        continue;
      }
      class_str[class_str_begin++] = c;
      
    } else {
      confidence_str[confidence_str_begin++] = c;
    }
  }
  Serial.println();
  confidence_str[confidence_str_begin] = 0;
}

void loop() {
  tft.fillScreen(0);
  tft.setCursor(20, 50);
  tft.setTextSize(1.8);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  tft.print(class_str);

  Serial.println(class_str);
  tft.setCursor(20, 90);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
  tft.print(confidence_str);

  delay(50);
}
