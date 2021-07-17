#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <SPI.h>
#include <Wire.h>

#define TFT_CS  10 // TFT LCD CS PIN
#define TFT_DC   8 // TFT DC(A0、RS) 
#define TFT_RST  9 // TFT Reset


Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
char status_str[20];
int has_defect = 0;
int status_str_index = 0;
String description_str;
String class_str[3];

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
  tft.fillScreen(0);
  
  while(Wire.available()){
    char c = Wire.read();
    status_str[status_str_index++] = c;
  }
  has_defect = (status_str[0] == '1');
  if (has_defect) {
    description_str = "Defect joint detected!\n";
    class_str[0] = "Insufficient: " + status_str[1] + "\n";
    class_str[1] = "Short: " + status_str[2] + "\n";
    class_str[2] = "Too much: " + status_str[3] + "\n";
  }
}

void loop() {
  if (!has_defect) {
    tft.setCursor(20, 50);
    tft.setTextSize(1.8);
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.print(description_str);
  } else {
    tft.setCursor(20, 30);
    tft.setTextSize(1.8);
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.print(description_str);

    for (int i = 0; i < 3; ++i) {
      tft.setCursor(20, 30 + 20 * (i + 1));
      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
      tft.print(class_str[i]);
    } 
  }

  delay(50);
}
