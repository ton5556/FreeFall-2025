#include <WiFiManager.h>
#include <WebServer.h>

WebServer server(80);

String t_value = "N/A";
String height_value = "N/A";

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, 4, 5);  // TX = GPIO5, RX = GPIO4 for ESP32-C3

  WiFiManager wifiManager;
  wifiManager.autoConnect("FreeFallAP");  // AP Mode if no WiFi saved

  server.on("/", HTTP_GET, []() {
    String html = "<html><head>";
    html += "<meta http-equiv='refresh' content='1'/>";
    html += "<title>Free Fall Measurement</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f4; padding: 20px; }";
    html += "h2 { color: #333; }";
    html += "p { font-size: 20px; font-weight: bold; color: #007bff; }";
    html += "div { background: white; padding: 20px; margin: 50px auto; width: 50%; border-radius: 10px; box-shadow: 0px 4px 10px rgba(0,0,0,0.2); }";
    html += "</style></head><body>";

    html += "<div>";
    html += "<h2>Free Fall Experiment</h2>";
    html += "<p>Time (s): " + String(t_value) + "</p>";
    html += "<p>Height (m): " + String(height_value) + "</p>";
    html += "</div>";

    html += "</body></html>";

    server.send(200, "text/html", html);
  });

  server.begin();
}

void loop() {
  server.handleClient();

  if (Serial1.available()) {
    String data = Serial1.readStringUntil('\n');
    int tIndex = data.indexOf("T:");
    int hIndex = data.indexOf("H:");

    if (tIndex != -1 && hIndex != -1) {
      t_value = data.substring(tIndex + 2, hIndex - 1);
      height_value = data.substring(hIndex + 2);
    }
  }
}
