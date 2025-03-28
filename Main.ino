#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// Define software serial pins
SoftwareSerial espSerial(0, 1);  // RX, TX (Connect to ESP32-C3 TX and RX)

// LCD setup
#define LCD_ADDRESS 0x23  // Change if needed
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);

void setup() {
  Serial.begin(9600);     // Debugging
  espSerial.begin(9600);  // Communication with ESP32-C3

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for IP...");
}

void loop() {
  static String receivedIP = "";

  while (espSerial.available()) {
    char c = espSerial.read();
    if (c == '\n') {  // IP is fully received
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ESP32 IP:");
      lcd.setCursor(0, 1);
      lcd.print(receivedIP);
      Serial.println("Received IP: " + receivedIP);  // Debugging
      receivedIP = "";                               // Reset for next IP
    } else {
      receivedIP += c;
    }
  }
}
