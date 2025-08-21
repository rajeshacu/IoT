#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_BMP085.h>
#include <TinyGPS++.h>

// ---------------- LoRa pins ----------------
#define LORA_SS   5
#define LORA_RST  14
#define LORA_DIO0 2

// ---------------- Push button pin ----------------
#define BUTTON_PIN 27

// ---------------- GPS pins ----------------
#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600

// ---------------- Battery estimation ----------------
unsigned long startTime;
const float batteryCapacity_mAh = 2500.0;   // 2x 18650 in series = 2500 mAh total
const float avgCurrent_mA = 200.0;          // Estimated avg current consumption (ESP32 + GPS + LoRa)

// ---------------- Create objects ----------------
Adafruit_BMP085 bmp;
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;

// ---------------- Battery estimation function ----------------
int getBatteryPercentage() {
  // Time elapsed in hours
  float elapsedHours = (millis() - startTime) / 3600000.0;

  // Runtime hours available based on capacity and current draw
  float runtimeHours = batteryCapacity_mAh / avgCurrent_mA;

  // Remaining percentage
  float percentageLeft = 100.0 - ((elapsedHours / runtimeHours) * 100.0);

  if (percentageLeft < 0) percentageLeft = 0;
  if (percentageLeft > 100) percentageLeft = 100;

  return (int)percentageLeft;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize battery timer
  startTime = millis();

  // Initialize BMP180
  if (!bmp.begin()) {
    Serial.println("Could not find BMP180 sensor!");
    while (1);
  }

  // Setup button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize GPS
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Waiting for GPS signal...");

  // Initialize LoRa
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("LoRa Sender Ready with Battery Estimation!");
}

void loop() {
  // Process GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Read BMP180 data
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0;
  float altitude = bmp.readAltitude();

  // Read button state (Active LOW)
  int buttonState = digitalRead(BUTTON_PIN);
  int alertFlag = (buttonState == LOW) ? 1 : 0;

  // Get estimated battery %
  int batteryPercent = getBatteryPercentage();

  // Create JSON based on GPS availability
  String jsonData = "{";
  jsonData += "\"id\":\"P1\",";
  jsonData += "\"temperature\":" + String(temperature, 1) + ",";
  jsonData += "\"pressure\":" + String(pressure, 1) + ",";
  jsonData += "\"altitude\":" + String(altitude, 1) + ",";

  if (gps.location.isValid()) {
    // If GPS has valid fix, include coordinates
    jsonData += "\"latitude\":" + String(gps.location.lat(), 6) + ",";
    jsonData += "\"longitude\":" + String(gps.location.lng(), 6) + ",";
    Serial.println("GPS FIX Available: Sending GPS + Other Data");
  } else {
    // If no GPS fix, send 0.000000 placeholders
    jsonData += "\"latitude\":0.000000,";
    jsonData += "\"longitude\":0.000000,";
    Serial.println("No GPS FIX: Sending Other Data Only");
  }

  // Add battery and alert
  jsonData += "\"battery\":" + String(batteryPercent) + ",";
  jsonData += "\"alert\":" + String(alertFlag);
  jsonData += "}";

  // Send JSON via LoRa
  LoRa.beginPacket();
  LoRa.print(jsonData);
  LoRa.endPacket();

  Serial.println("Sent: " + jsonData);
  Serial.println("--------------------");

  delay(5000); // Send every 5 seconds
}
