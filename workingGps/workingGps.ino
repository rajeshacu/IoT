#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_BMP085.h>
#include <TinyGPS++.h>

// LoRa pins
#define LORA_SS   5
#define LORA_RST  14
#define LORA_DIO0 2

// Push button pin
#define BUTTON_PIN 27

// GPS pins
#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600

// Create objects
Adafruit_BMP085 bmp;
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  while (!Serial);

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

  Serial.println("LoRa Sender with GPS + BMP180 Ready!");
}

void loop() {
  // Process GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Send data only when GPS location updates
  if (gps.location.isUpdated()) {
    double latitude = gps.location.lat();
    double longitude = gps.location.lng();

    // Read BMP180 data
    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0;
    float altitude = bmp.readAltitude();

    // Read button state (Active LOW)
    int buttonState = digitalRead(BUTTON_PIN);
    int alertFlag = (buttonState == LOW) ? 1 : 0;

    // Create JSON string
    String jsonData = "{";
    jsonData += "\"id\":\"P1\",";
    jsonData += "\"temperature\":" + String(temperature, 1) + ",";
    jsonData += "\"pressure\":" + String(pressure, 1) + ",";
    jsonData += "\"altitude\":" + String(altitude, 1) + ",";
    jsonData += "\"latitude\":" + String(latitude, 6) + ",";
    jsonData += "\"longitude\":" + String(longitude, 6) + ",";
    jsonData += "\"battery\":80,";
    jsonData += "\"alert\":" + String(alertFlag);
    jsonData += "}";

    // Send JSON via LoRa
    LoRa.beginPacket();
    LoRa.print(jsonData);
    LoRa.endPacket();

    Serial.println("Sent: " + jsonData);
    Serial.println("--------------------");
  }
}
