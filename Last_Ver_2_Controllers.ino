#include <Wire.h>
#include <Adafruit_VL6180X.h>

// Pin Definitions
#define LED_PIN1 13
#define LED_PIN2 12
#define SHTD_PIN1 2
#define SHTD_PIN2 3

// Optimization Constants
const float DISTANCE_THRESHOLD = 10.0; // Distance threshold in cm
const uint8_t MULTIPLEXER_ADDR = 0x70;  // PCA9548A address
const uint8_t SENSOR1_CHANNEL = 0;      // Multiplexer channel for sensor 1
const uint8_t SENSOR2_CHANNEL = 1;      // Multiplexer channel for sensor 2
const unsigned long MEASUREMENT_INTERVAL = 10; // Reduced interval for faster detection

// Global variables
Adafruit_VL6180X sensor1 = Adafruit_VL6180X();
Adafruit_VL6180X sensor2 = Adafruit_VL6180X();
unsigned long lastMeasurementTime = 0;

// Optimized multiplexer channel selection
void selectMultiplexerChannel(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(MULTIPLEXER_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

// Robust sensor initialization
bool initializeSensor(Adafruit_VL6180X &sensor, uint8_t channel) {
  selectMultiplexerChannel(channel);

  for (int attempt = 0; attempt < 3; attempt++) {
    if (sensor.begin()) {
      return true;
    }
    delay(50);
  }
  return false;
}

void setup() {
  // Minimal setup for fast performance
  Wire.begin();

  // Configure pins
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(SHTD_PIN1, OUTPUT);
  pinMode(SHTD_PIN2, OUTPUT);

  // Power up sequence
  digitalWrite(SHTD_PIN1, HIGH);
  digitalWrite(SHTD_PIN2, HIGH);
  delay(50);

  // Initialize sensors
  if (!initializeSensor(sensor1, SENSOR1_CHANNEL) ||
      !initializeSensor(sensor2, SENSOR2_CHANNEL)) {
    // Emergency halt if sensors fail
    while (1) {
      digitalWrite(LED_PIN1, HIGH);
      digitalWrite(LED_PIN2, HIGH);
      delay(500);
      digitalWrite(LED_PIN1, LOW);
      digitalWrite(LED_PIN2, LOW);
      delay(500);
    }
  }
}

void loop() {
  unsigned long currentTime = millis();

  // Rapid measurement cycle
  if (currentTime - lastMeasurementTime >= MEASUREMENT_INTERVAL) {
    // Read Sensor 1
    selectMultiplexerChannel(SENSOR1_CHANNEL);
    uint8_t range1 = sensor1.readRange();
    uint8_t status1 = sensor1.readRangeStatus();

    // Read Sensor 2
    selectMultiplexerChannel(SENSOR2_CHANNEL);
    uint8_t range2 = sensor2.readRange();
    uint8_t status2 = sensor2.readRangeStatus();

    // Rapid LED Control
    digitalWrite(LED_PIN1,
                 (status1 == VL6180X_ERROR_NONE && (range1 / 10.0) <= DISTANCE_THRESHOLD)
                );

    digitalWrite(LED_PIN2,
                 (status2 == VL6180X_ERROR_NONE && (range2 / 10.0) <= DISTANCE_THRESHOLD)
                );

    // Update last measurement time
    lastMeasurementTime = currentTime;
  }
}
