#include <WiFiManager.h>
#include <WebServer.h>
#include <WiFi.h>

WebServer server(80);

String t = "N/A";
String distance_m_fig = "N/A";

const int resetPin = 8; // Use a valid pin for WiFi reset

// Store previous measurements
const int historySize = 5;
String t_history[historySize];
String h_history[historySize];

// Device status tracking
unsigned long uptime = 0;
unsigned long lastMeasurementTime = 0;
int measurementCount = 0;
float wifiSignalStrength = 0;
String ipAddress = "";

// Static IP Configuration
IPAddress local_IP(192, 168, 1, 200);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, 5, 6);  // TX = GPIO5, RX = GPIO4 for ESP32-C3

  pinMode(resetPin, INPUT_PULLUP);

  WiFiManager wifiManager;

  if (digitalRead(resetPin) == LOW) {
    Serial.println("Resetting WiFi settings...");
    wifiManager.resetSettings();
    delay(1000);
    ESP.restart();
  }

  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Failed to configure Static IP.");
  }

  if (!wifiManager.autoConnect("FreeFallAP")) {
    Serial.println("Failed to connect to WiFi. Restarting...");
    delay(1000);
    ESP.restart();
  }

  ipAddress = WiFi.localIP().toString();
  Serial.println("WiFi connected!");
  Serial.print("Access the web interface at: http://");
  Serial.println(ipAddress);

  // Webpage
  server.on("/", HTTP_GET, []() {
    // Update status information
    uptime = millis() / 1000; // Convert to seconds
    wifiSignalStrength = WiFi.RSSI();

    String html = "<!DOCTYPE html>"
                  "<html lang=\"en\">"
                  "<head>"
                  "    <meta http-equiv=\"refresh\" content=\"5\"/>"
                  "    <title>Device Status Dashboard</title>"
                  "    <style>"
                  "        :root {"
                  "            --primary-color: #3498db;"
                  "            --secondary-color: #2ecc71;"
                  "            --accent-color: #e74c3c;"
                  "            --dark-color: #2c3e50;"
                  "            --light-color: #ecf0f1;"
                  "            --warning-color: #f39c12;"
                  "            --success-color: #27ae60;"
                  "        }"
                  "        "
                  "        body {"
                  "            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;"
                  "            background: linear-gradient(135deg, var(--light-color) 0%, #bdc3c7 100%);"
                  "            margin: 0;"
                  "            padding: 20px;"
                  "            color: var(--dark-color);"
                  "            min-height: 100vh;"
                  "        }"
                  "        "
                  "        .container {"
                  "            background-color: white;"
                  "            border-radius: 10px;"
                  "            box-shadow: 0 10px 20px rgba(0, 0, 0, 0.1);"
                  "            padding: 25px;"
                  "            margin-bottom: 30px;"
                  "            max-width: 800px;"
                  "            margin-left: auto;"
                  "            margin-right: auto;"
                  "        }"
                  "        "
                  "        h2 {"
                  "            color: var(--primary-color);"
                  "            border-bottom: 2px solid var(--primary-color);"
                  "            padding-bottom: 10px;"
                  "            margin-top: 0;"
                  "        }"
                  "        "
                  "        .status-grid {"
                  "            display: grid;"
                  "            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));"
                  "            gap: 20px;"
                  "            margin: 20px 0;"
                  "        }"
                  "        "
                  "        .status-card {"
                  "            background-color: var(--light-color);"
                  "            border-radius: 8px;"
                  "            padding: 15px;"
                  "            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.05);"
                  "            text-align: center;"
                  "            transition: transform 0.3s ease;"
                  "        }"
                  "        "
                  "        .status-card:hover {"
                  "            transform: translateY(-5px);"
                  "        }"
                  "        "
                  "        .status-icon {"
                  "            font-size: 32px;"
                  "            margin-bottom: 10px;"
                  "        }"
                  "        "
                  "        .status-title {"
                  "            font-size: 14px;"
                  "            color: var(--dark-color);"
                  "            margin-bottom: 5px;"
                  "            font-weight: 600;"
                  "        }"
                  "        "
                  "        .status-value {"
                  "            font-size: 24px;"
                  "            font-weight: bold;"
                  "            color: var(--primary-color);"
                  "        }"
                  "        "
                  "        .good {"
                  "            color: var(--success-color);"
                  "        }"
                  "        "
                  "        .warning {"
                  "            color: var(--warning-color);"
                  "        }"
                  "        "
                  "        .critical {"
                  "            color: var(--accent-color);"
                  "        }"
                  "        "
                  "        .measurement {"
                  "            font-size: 18px;"
                  "            margin: 15px 0;"
                  "            display: flex;"
                  "            justify-content: space-between;"
                  "            background-color: var(--light-color);"
                  "            padding: 12px 20px;"
                  "            border-radius: 8px;"
                  "            align-items: center;"
                  "        }"
                  "        "
                  "        .measurement span {"
                  "            font-weight: bold;"
                  "            color: var(--primary-color);"
                  "            font-size: 24px;"
                  "        }"
                  "        "
                  "        table {"
                  "            width: 100%;"
                  "            border-collapse: collapse;"
                  "            margin: 20px 0;"
                  "        }"
                  "        "
                  "        th, td {"
                  "            padding: 12px 15px;"
                  "            text-align: center;"
                  "            border-bottom: 1px solid #ddd;"
                  "        }"
                  "        "
                  "        th {"
                  "            background-color: var(--primary-color);"
                  "            color: white;"
                  "        }"
                  "        "
                  "        tr:nth-child(even) {"
                  "            background-color: #f2f2f2;"
                  "        }"
                  "        "
                  "        tr:hover {"
                  "            background-color: #e2f1ff;"
                  "        }"
                  "        "
                  "        button {"
                  "            background-color: var(--accent-color);"
                  "            color: white;"
                  "            border: none;"
                  "            padding: 12px 20px;"
                  "            border-radius: 5px;"
                  "            cursor: pointer;"
                  "            font-size: 16px;"
                  "            transition: all 0.3s ease;"
                  "            margin-top: 10px;"
                  "        }"
                  "        "
                  "        button:hover {"
                  "            background-color: #c0392b;"
                  "            transform: translateY(-2px);"
                  "            box-shadow: 0 5px 10px rgba(0, 0, 0, 0.1);"
                  "        }"
                  "        "
                  "        .header {"
                  "            text-align: center;"
                  "            margin-bottom: 30px;"
                  "        }"
                  "        "
                  "        .logo {"
                  "            font-size: 36px;"
                  "            font-weight: bold;"
                  "            color: var(--primary-color);"
                  "            text-shadow: 2px 2px 4px rgba(0,0,0,0.2);"
                  "            margin-bottom: 10px;"
                  "        }"
                  "        "
                  "        .subheader {"
                  "            color: var(--dark-color);"
                  "            font-size: 18px;"
                  "        }"
                  "        "
                  "        .progress-bar {"
                  "            width: 100%;"
                  "            background-color: #e0e0e0;"
                  "            border-radius: 10px;"
                  "            margin: 10px 0;"
                  "            overflow: hidden;"
                  "        }"
                  "        "
                  "        .progress {"
                  "            height: 10px;"
                  "            border-radius: 10px;"
                  "        }"
                  "        "
                  "        .wifi-details {"
                  "            display: flex;"
                  "            flex-direction: column;"
                  "            gap: 15px;"
                  "        }"
                  "        "
                  "        .detail-row {"
                  "            display: flex;"
                  "            justify-content: space-between;"
                  "            align-items: center;"
                  "            background-color: var(--light-color);"
                  "            padding: 12px 20px;"
                  "            border-radius: 8px;"
                  "        }"
                  "        "
                  "        .detail-value {"
                  "            font-weight: bold;"
                  "            color: var(--primary-color);"
                  "        }"
                  "        "
                  "        .wifi-controls {"
                  "            display: flex;"
                  "            gap: 15px;"
                  "            margin-top: 20px;"
                  "            justify-content: center;"
                  "        }"
                  "        "
                  "        .wifi-btn {"
                  "            padding: 12px 20px;"
                  "            border-radius: 5px;"
                  "            cursor: pointer;"
                  "            font-size: 16px;"
                  "            transition: all 0.3s ease;"
                  "            border: none;"
                  "            font-weight: 600;"
                  "        }"
                  "        "
                  "        .primary-btn {"
                  "            background-color: var(--primary-color);"
                  "            color: white;"
                  "        }"
                  "        "
                  "        .primary-btn:hover {"
                  "            background-color: #2980b9;"
                  "            transform: translateY(-2px);"
                  "            box-shadow: 0 5px 10px rgba(0, 0, 0, 0.1);"
                  "        }"
                  "        "
                  "        @media (max-width: 768px) {"
                  "            .container {"
                  "                padding: 15px;"
                  "            }"
                  "            "
                  "            .status-grid {"
                  "                grid-template-columns: 1fr;"
                  "            }"
                  "            "
                  "            .measurement {"
                  "                font-size: 16px;"
                  "            }"
                  "            "
                  "            .measurement span {"
                  "                font-size: 20px;"
                  "            }"
                  "            "
                  "            .wifi-controls {"
                  "                flex-direction: column;"
                  "            }"
                  "        }"
                  "    </style>"
                  "</head>"
                  "<body>"
                  "    <div class=\"header\">"
                  "        <div class=\"logo\">Device Dashboard</div>"
                  "        <div class=\"subheader\">ESP32-C3 Status Monitor</div>"
                  "    </div>"
                  "    "
                  "    <div class=\"container\">"
                  "        <h2>System Status</h2>"
                  "        <div class=\"status-grid\">"
                  "            <div class=\"status-card\">"
                  "                <div class=\"status-icon\"></div>"
                  "                <div class=\"status-title\">UPTIME</div>"
                  "                <div class=\"status-value\">" + formatUptime(uptime) + "</div>"
                  "            </div>"
                  "            <div class=\"status-card\">"
                  "                <div class=\"status-title\">WIFI SIGNAL</div>";

    // Signal strength indicator with color coding
    if (wifiSignalStrength > -50) {
      html += "                <div class=\"status-value good\">" + String(wifiSignalStrength) + " dBm</div>";
    } else if (wifiSignalStrength > -70) {
      html += "                <div class=\"status-value warning\">" + String(wifiSignalStrength) + " dBm</div>";
    } else {
      html += "                <div class=\"status-value critical\">" + String(wifiSignalStrength) + " dBm</div>";
    }

    html += "                <div class=\"progress-bar\">"
            "                    <div class=\"progress\" style=\"width: " + String(map(constrain(wifiSignalStrength, -90, -30), -90, -30, 0, 100)) + "%; background-color: " + getSignalColor(wifiSignalStrength) + ";\"></div>"
            "                </div>"
            "            </div>"
            "            <div class=\"status-card\">"
            "                <div class=\"status-title\">LAST UPDATE</div>"
            "                <div class=\"status-value\">" + getLastUpdateTime() + "</div>"
            "            </div>"
            "            <div class=\"status-card\">"
            "                <div class=\"status-title\">IP ADDRESS</div>"
            "                <div class=\"status-value\" style=\"font-size: 18px;\">" + ipAddress + "</div>"
            "            </div>"
            "        </div>"
            "    </div>"
            "    "
            "    <div class=\"container\">"
            "        <h2>Current Measurements</h2>"
            "        "
            "        <div class=\"measurement\">"
            "            <strong>Time (s):</strong> <span id=\"time\">" + t + "</span>"
            "        </div>"
            "        <div class=\"measurement\">"
            "            <strong>Height (m):</strong> <span id=\"height\">" + distance_m_fig + "</span>"
            "        </div>"
            "    </div>"
            "    "
            "    <div class=\"container\">"
            "        <h2>Measurement History</h2>"
            "        <table id=\"records-table\">"
            "            <tr>"
            "                <th>Time (s)</th>"
            "                <th>Height (m)</th>"
            "                <th>Calculated g (m/s²)</th>"
            "            </tr>";

    // Add history entries to the table
    for (int i = 0; i < historySize; i++) {
      if (t_history[i] != "") {
        float time = t_history[i].toFloat();
        float height = h_history[i].toFloat();
        float g = 0;

        // Calculate g if we have valid numbers
        if (time > 0 && !isnan(time) && !isnan(height)) {
          g = (2 * height) / (time * time);
        }

        html += "<tr><td>" + t_history[i] + "</td><td>" + h_history[i] + "</td><td>" + String(g, 3) + "</td></tr>";
      }
    }

    html += "        </table>"
            "        <form action=\"/clear\" method=\"get\">"
            "            <button type=\"submit\">Clear History</button>"
            "        </form>"
            "    </div>"
            "    "
            "    <div class=\"container\">"
            "        <h2>Wi-Fi Connection</h2>"
            "        <div class=\"wifi-details\">"
            "            <div class=\"detail-row\">"
            "                <strong>SSID:</strong>"
            "                <span class=\"detail-value\">" + WiFi.SSID() + "</span>"
            "            </div>"
            "            <div class=\"detail-row\">"
            "                <strong>Signal Strength:</strong>"
            "                <span class=\"detail-value " + getSignalClass(wifiSignalStrength) + "\">" + String(wifiSignalStrength) + " dBm (" + getSignalQuality(wifiSignalStrength) + ")</span>"
            "            </div>"
            "            <div class=\"detail-row\">"
            "                <strong>MAC Address:</strong>"
            "                <span class=\"detail-value\">" + WiFi.macAddress() + "</span>"
            "            </div>"
            "            <div class=\"detail-row\">"
            "                <strong>IP Address:</strong>"
            "                <span class=\"detail-value\">" + ipAddress + "</span>"
            "            </div>"
            "            <div class=\"detail-row\">"
            "                <strong>Gateway:</strong>"
            "                <span class=\"detail-value\">" + WiFi.gatewayIP().toString() + "</span>"
            "            </div>"
            "            <div class=\"detail-row\">"
            "                <strong>Subnet Mask:</strong>"
            "                <span class=\"detail-value\">" + WiFi.subnetMask().toString() + "</span>"
            "            </div>"
            "            <div class=\"detail-row\">"
            "                <strong>DNS Server:</strong>"
            "                <span class=\"detail-value\">" + WiFi.dnsIP().toString() + "</span>"
            "            </div>"
            "        </div>"
            "        <div class=\"wifi-controls\">"
            "            <form action=\"/reset-wifi\" method=\"get\">"
            "                <button type=\"submit\" class=\"wifi-btn primary-btn\">Reset Wi-Fi Settings</button>"
            "            </form>"
            "            <form action=\"/restart\" method=\"get\">"
            "                <button type=\"submit\" class=\"wifi-btn\">Restart Device</button>"
            "            </form>"
            "        </div>"
            "    </div>"
            "</body>"
            "</html>";

    server.send(200, "text/html", html);
  });

  // Clear History Button
  server.on("/clear", HTTP_GET, []() {
    for (int i = 0; i < historySize; i++) {
      t_history[i] = "";
      h_history[i] = "";
    }
    t = "N/A";
    distance_m_fig = "N/A";
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "History cleared. Redirecting...");
  });

  // Wi-Fi Reset Button
  server.on("/reset-wifi", HTTP_GET, []() {
    server.send(200, "text/html",
                "<html><body>"
                "<h2>Resetting Wi-Fi settings...</h2>"
                "<p>The device will restart and create a new access point.</p>"
                "<p>Connect to 'FreeFallAP' to configure your Wi-Fi settings.</p>"
                "<script>setTimeout(function(){ window.close(); }, 5000);</script>"
                "</body></html>");

    delay(1000);
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    delay(1000);
    ESP.restart();
  });

  // Restart Device Button
  server.on("/restart", HTTP_GET, []() {
    server.send(200, "text/html",
                "<html><body>"
                "<h2>Restarting device...</h2>"
                "<p>The device will restart in a few seconds.</p>"
                "<p>Please wait while the device boots up.</p>"
                "<script>setTimeout(function(){ window.close(); }, 5000);</script>"
                "</body></html>");

    delay(1000);
    ESP.restart();
  });

  server.onNotFound([]() {
    server.send(404, "text/plain", "Page not found");
  });

  server.begin();
}

void loop() {
  server.handleClient();

  if (Serial1.available()) {
    String data = Serial1.readStringUntil('\n');
    data.trim();

    int tIndex = data.indexOf("T:");
    int hIndex = data.indexOf("H:");

    if (tIndex != -1 && hIndex != -1) {
      String temp_t = data.substring(tIndex + 2, hIndex - 1);
      String temp_h = data.substring(hIndex + 2);

      if (temp_t.length() > 0 && temp_h.length() > 0) {
        // Shift history down to make space for the new value
        for (int i = historySize - 1; i > 0; i--) {
          t_history[i] = t_history[i - 1];
          h_history[i] = h_history[i - 1];
        }

        // Store the latest measurement at index 0
        t_history[0] = t;
        h_history[0] = distance_m_fig;

        // Update the latest values
        t = temp_t;
        distance_m_fig = temp_h;

        // Update measurement tracking
        lastMeasurementTime = millis();
        measurementCount++;
      }
    }
  }

  if (digitalRead(resetPin) == LOW) {
    Serial.println("WiFi Reset button pressed! Resetting WiFi settings...");
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    delay(1000);
    ESP.restart();
  }
}

// Helper functions for formatting display values
String formatUptime(unsigned long seconds) {
  int days = seconds / 86400;
  seconds %= 86400;
  int hours = seconds / 3600;
  seconds %= 3600;
  int minutes = seconds / 60;
  seconds %= 60;

  if (days > 0) {
    return String(days) + "d " + String(hours) + "h";
  } else if (hours > 0) {
    return String(hours) + "h " + String(minutes) + "m";
  } else if (minutes > 0) {
    return String(minutes) + "m " + String(seconds) + "s";
  } else {
    return String(seconds) + "s";
  }
}

String getLastUpdateTime() {
  if (lastMeasurementTime == 0) {
    return "No data yet";
  }

  unsigned long timeSinceUpdate = (millis() - lastMeasurementTime) / 1000;

  if (timeSinceUpdate < 60) {
    return String(timeSinceUpdate) + "s ago";
  } else if (timeSinceUpdate < 3600) {
    return String(timeSinceUpdate / 60) + "m ago";
  } else {
    return String(timeSinceUpdate / 3600) + "h ago";
  }
}

String getSignalColor(float signalStrength) {
  if (signalStrength > -50) {
    return "#27ae60"; // Strong - Green
  } else if (signalStrength > -70) {
    return "#f39c12"; // Medium - Orange
  } else {
    return "#e74c3c"; // Weak - Red
  }
}

String getSignalClass(float signalStrength) {
  if (signalStrength > -50) {
    return "good"; // Strong
  } else if (signalStrength > -70) {
    return "warning"; // Medium
  } else {
    return "critical"; // Weak
  }
}

String getSignalQuality(float signalStrength) {
  if (signalStrength > -50) {
    return "Excellent";
  } else if (signalStrength > -60) {
    return "Good";
  } else if (signalStrength > -70) {
    return "Fair";
  } else if (signalStrength > -80) {
    return "Poor";
  } else {
    return "Very Poor";
  }
}
