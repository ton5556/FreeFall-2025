#include <Wire.h>
#include <VL53L0X.h>
#include <LiquidCrystal_I2C.h>

#define IR_Sensor1 2
#define IR_Sensor2 3
#define Ledred 13
#define Ledgreen 10
#define MAX_DISTANCE 600

TwoWire lcdWire;
VL53L0X sensor;
LiquidCrystal_I2C lcd(0x27, 16, 2);

volatile unsigned long startMicros = 0;
volatile unsigned long endMicros = 0;

volatile bool measurementComplete = false;
volatile bool firstSensorTriggered = false;
volatile bool secondSensorTriggered = false;
volatile bool bothSensorsTriggered = false;

double g = 9.81;
double t;

void setup() {
  Serial.begin(9600);   // Debugging
  Serial1.begin(9600);  // Communication with ESP32

  pinMode(IR_Sensor1, INPUT);
  pinMode(IR_Sensor2, INPUT);
  pinMode(Ledred, OUTPUT);
  pinMode(Ledgreen, OUTPUT);

  lcd.begin(16, 2, &lcdWire);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Free Fall");
  lcd.setCursor(0, 1);
  lcd.print("Height (m):");
  delay(3000);

  digitalWrite(Ledgreen, HIGH);

  Wire.begin();
  sensor.init();
  sensor.setTimeout(500);

  attachInterrupt(digitalPinToInterrupt(IR_Sensor1), ISR_IR_Sensor1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(IR_Sensor2), ISR_IR_Sensor2, CHANGE);
}

void loop() {
  static unsigned long lastUpdateTime = 0;
  static const unsigned long updateInterval = 500;  // 500ms interval for LCD updates
  unsigned long currentTime = millis();

  int distance_mm = sensor.readRangeSingleMillimeters();
  float distance_m = distance_mm / 1000.0;
  float distance_m_fig = distance_m - 0.03;  // Adjust for sensor offset

  if (currentTime - lastUpdateTime >= updateInterval) {
    if (!measurementComplete) {
      updateLCD(distance_m_fig);
    }
    lastUpdateTime = currentTime;
  }

  t = sqrt((2 * distance_m_fig) / g);

  if (measurementComplete) {
    Serial.print("T:"); Serial.print(t, 4);
    Serial.print(",H:"); Serial.println(distance_m_fig, 4);

    Serial1.print("T:"); 
    Serial1.print(t, 4); 
    Serial1.print(",H:"); 
    Serial1.println(distance_m_fig, 4);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Time (s):");
    lcd.setCursor(10, 0);
    lcd.print(t, 4);

    lcd.setCursor(0, 1);
    lcd.print("Height (m):");
    lcd.setCursor(10, 1);
    lcd.print(distance_m_fig, 4);

    delay(2000);
    measurementComplete = false;
    firstSensorTriggered = false;
    secondSensorTriggered = false;
    digitalWrite(Ledgreen, HIGH);
  }
}

void ISR_IR_Sensor1() {
  if (digitalRead(IR_Sensor1) == HIGH) {
    startMicros = micros();
    firstSensorTriggered = true;
    digitalWrite(Ledred, HIGH);
    digitalWrite(Ledgreen, LOW);
    if (firstSensorTriggered && secondSensorTriggered) {
      bothSensorsTriggered = true;
    }
  }
}

void ISR_IR_Sensor2() {
  if (digitalRead(IR_Sensor2) == LOW) {
    if (firstSensorTriggered) {
      endMicros = micros();
      measurementComplete = true;
      digitalWrite(Ledred, LOW);
      digitalWrite(Ledgreen, HIGH);
      secondSensorTriggered = true;
      if (firstSensorTriggered && secondSensorTriggered) {
        bothSensorsTriggered = true;
      }
    }
  }
}

void updateLCD(float distance_m_fig) {
  lcd.setCursor(0, 1);
  lcd.print("Height (m):");
  lcd.setCursor(10, 1);
  lcd.print(distance_m_fig, 4);
}
