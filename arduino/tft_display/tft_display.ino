#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <SPI.h>
#include <Wire.h>
#include <string.h>

#define TFT_DC   7 // TFT DC(A0、RS) 
#define TFT_RST  8 // TFT Reset
#define TFT_CS   9 // TFT LCD CS PIN

#define VRX A0
#define VRY A1
#define GPIO0 6
#define GPIO1 4
#define GPIO2 5


Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
int has_defect = 0;
char received_str[5][5];
char display_str[4][20] = {0};
unsigned long last;

/* Joystick state
  0: neutral
  1: up
  2: down
  3: left
  4: right
*/
int8_t current_joy_state = 0;
int8_t state_set = 0;
uint16_t position_x = 0;
uint16_t position_y = 0;

void setup(void) {
  
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab

  tft.setRotation(1);
  tft.setCursor(5, 30);
  tft.setTextColor(ST7735_WHITE);
  tft.setFont(&FreeSerif12pt7b);
  tft.print("Joint Detection");
  tft.setCursor(5, 60);
  tft.setTextColor(ST7735_RED);
  tft.setFont(&FreeSansBold9pt7b);
  tft.print("I2C Not Working");
  tft.setCursor(0, 80);
  tft.setTextColor(ST7735_YELLOW);
  tft.setFont(&FreeSans9pt7b);

  // pinMode(VRX, INPUT);
  // pinMode(VRY, INPUT);
  pinMode(GPIO0, OUTPUT);
  pinMode(GPIO1, OUTPUT);
  pinMode(GPIO2, OUTPUT);

  Wire.begin(0x12);
  Wire.onReceive(receiveEvent);
  Serial.begin(57600);
  Serial.println("I2C Slave Started");
  tft.fillScreen(0);

  last = millis();
}

// i2c received signal handler
void receiveEvent(int numBytes){
  
  Serial.println("received");                                                                                                                                         
  tft.fillScreen(ST7735_BLACK);
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
  
  // acknowledge of joystick signal
  if (received_str[0][0] == '1') {
    state_set = 0;
    current_joy_state = 0;
    return;
  }
  
  // decode received data to display strings
  has_defect = (received_str[1][0] == '1');
  if (has_defect) {
    sprintf(display_str[0], "Defect detected!");
  } else {
    sprintf(display_str[0], "QC PASSED !!!");
  }
  sprintf(display_str[1], "Short: %s", received_str[3]);
  sprintf(display_str[2], "Excess", received_str[4]);
  sprintf(display_str[3], "Poor filled: %s", received_str[2]);

  Serial.println(display_str[0]);
}

void loop() {
  position_x = analogRead(VRX);
  position_y = analogRead(VRY);
  Serial.print("VRX: ");
  Serial.print(position_x);
  Serial.print(" VRY: ");
  Serial.println(position_y);
  
  // determine joystick state
  if (!state_set) {
    current_joy_state = 0;
    if (position_x > 950) {
      state_set = 1;
      current_joy_state = 1;
    }
    if (position_x < 70) {
      state_set = 1;
      current_joy_state = 2;
    }
    if (position_y > 950) {
      state_set = 1;
      current_joy_state = 3;
    }
    if (position_y < 70) {
      state_set = 1;
      current_joy_state = 4;
    }
  }

  // if state is set, keep sending signal until ack
  if (state_set) {
    switch (current_joy_state) {
      case 1:
       digitalWrite(GPIO0, LOW); 
       digitalWrite(GPIO1, LOW); 
       digitalWrite(GPIO2, HIGH); 
       break;
      case 2:
       digitalWrite(GPIO0, LOW); 
       digitalWrite(GPIO1, HIGH); 
       digitalWrite(GPIO2, LOW); 
       break;
      case 3:
       digitalWrite(GPIO0, LOW); 
       digitalWrite(GPIO1, HIGH); 
       digitalWrite(GPIO2, HIGH); 
       break;
      case 4:
       digitalWrite(GPIO0, HIGH); 
       digitalWrite(GPIO1, LOW); 
       digitalWrite(GPIO2, LOW); 
       break;
    }
  } else {
   digitalWrite(GPIO0, LOW); 
   digitalWrite(GPIO1, LOW); 
   digitalWrite(GPIO2, LOW); 
  }
  

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
    tft.setTextColor(ST7735_RED, ST7735_BLACK);
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
