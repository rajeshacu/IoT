#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_BMP085.h>
#include <TinyGPS++.h>

// LoRa Pins
#define LORA_SS   5
#define LORA_RST  14
#define LORA_DIO0 2

// Button
#define BUTTON_PIN 27

// GPS Pins
#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600

// Battery estimation
int batteryPercent = 100;
unsigned long lastBatteryDropTime = 0;
const unsigned long dropInterval = 7.5 * 60 * 1000UL;

int getBatteryPercentage() {
  if (millis() - lastBatteryDropTime >= dropInterval) {
    if (batteryPercent > 0) batteryPercent--;
    lastBatteryDropTime = millis();
  }
  return batteryPercent;
}

// Sensors
Adafruit_BMP085 bmp;
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!bmp.begin()) {
    Serial.println("Could not find BMP180 sensor!");
    while (1);
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("P2 Node Ready");

  delay(2500); // Offset send by 2.5 seconds to avoid collision
}

void loop() {
  while (gpsSerial.available() > 0) gps.encode(gpsSerial.read());

  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0;
  float altitude = bmp.readAltitude();
  int alertFlag = (digitalRead(BUTTON_PIN) == LOW) ? 1 : 0;
  int battery = getBatteryPercentage();

  String jsonData = "{";
  jsonData += "\"id\":\"P2\",";
  jsonData += "\"temperature\":" + String(temperature, 1) + ",";
  jsonData += "\"pressure\":" + String(pressure, 1) + ",";
  jsonData += "\"altitude\":" + String(altitude, 1) + ",";

  if (gps.location.isValid()) {
    jsonData += "\"latitude\":" + String(gps.location.lat(), 6) + ",";
    jsonData += "\"longitude\":" + String(gps.location.lng(), 6) + ",";
  } else {
    jsonData += "\"latitude\":0.000000,";
    jsonData += "\"longitude\":0.000000,";
  }

  jsonData += "\"battery\":" + String(battery) + ",";
  jsonData += "\"alert\":" + String(alertFlag);
  jsonData += "}";

  LoRa.beginPacket();
  LoRa.print(jsonData);
  LoRa.endPacket();

  Serial.println("Sent P2: " + jsonData);
  delay(5000); // Send every 5 sec
}
