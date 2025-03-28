#include <WiFiManager.h>
#include <WiFi.h>

WiFiManager wm;

#define RESET_PIN 8  // Define WiFi reset button pin

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, 5, 6);  // TX: GPIO 5, RX: GPIO 6

  pinMode(RESET_PIN, INPUT_PULLUP);

  if (digitalRead(RESET_PIN) == LOW) {
    Serial.println("Resetting WiFi...");
    wm.resetSettings();
    delay(2000);
    ESP.restart();
  }

  if (!wm.autoConnect("ESP32_AP")) {
    Serial.println("Failed to connect");
    ESP.restart();
  }

  delay(3000);  // Ensure stable WiFi before sending IP

  if (WiFi.status() == WL_CONNECTED) {
    String ipAddress = WiFi.localIP().toString();
    Serial.println("ESP32 IP: " + ipAddress);

    Serial1.println(ipAddress);  // ✅ Send IP to Arduino Uno ✅
  } else {
    Serial.println("WiFi connection failed!");
    Serial1.println("No WiFi");
  }
}

void loop() {
  // Nothing needed here for now
}
