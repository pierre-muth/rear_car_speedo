#include <Arduino.h>
#include <U8g2lib.h>

#define RXD1 12
#define TXD1 13
#define RXD2 16
#define TXD2 17

U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C u8g2(U8G2_R2, U8X8_PIN_NONE, 22, 21);
String buildINString="test";
String buildString = "";
String subString = "";
String speedString = "";
int speed = 42;
float correctedSpeed = 42.0;
char speedValueChars [3] = {'*', '*', '*'};

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  u8g2.begin();

  Serial.println("Starting ...");
}

void loop() {
  Serial.println(".");

  Serial2.println("010d");
  delay(100);

  buildString = "";
  buildINString = "";
  while (Serial2.available()) {
    char inChar = (char)Serial2.read();
    if (inChar > 0x1F) {
      buildString += inChar;
      buildINString += inChar;
    }
    delay(1);
  }

  Serial.println(buildINString);

  while (buildString.length() > 7) {
    if (buildString.startsWith("41 0D")) {
      speedString = buildString.charAt(6);
      speedString += buildString.charAt(7);
      speed = strtol(speedString.c_str(), NULL, 16);
      correctedSpeed = speed + (speed*0.04);
      speed = (int)correctedSpeed;  
      itoa(speed, speedValueChars, 10);
      break;
    } else {
      speedValueChars[0] = '-';
      speedValueChars[1] = '-';
      speedValueChars[2] = '-';
      // speed = 48;
      // correctedSpeed = speed + (speed*0.05);
      // speed = (int)correctedSpeed;  
      // itoa(speed, speedValueChars, 10);
    }
    buildString = buildString.substring(1);
  }

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_5x7_mf);
    u8g2.drawStr(0,6, buildINString.c_str());
    u8g2.setFont(u8g2_font_profont29_mf);
    u8g2.drawStr(4,30, speedValueChars);
    u8g2.drawStr(60,30, "Km/h");
  } while ( u8g2.nextPage() );

  Serial1.print((char)speed);


  delay(400);
}