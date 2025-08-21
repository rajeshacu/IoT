#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_BMP085.h>

// LoRa pins
#define LORA_SS   5
#define LORA_RST  14
#define LORA_DIO0 2

// Push button pin
#define BUTTON_PIN 27

// Create BMP180 object
Adafruit_BMP085 bmp;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Initialize BMP180
  if (!bmp.begin()) {
    Serial.println("Could not find BMP180 sensor!");
    while (1);
  }

  // Setup button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Using internal pull-up

  // Initialize LoRa
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("LoRa Sender with Alert Ready!");
}

void loop() {
  // Read BMP180 data
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0;
  float altitude = bmp.readAltitude();

  // Read button state
  int buttonState = digitalRead(BUTTON_PIN);
  int alertFlag = (buttonState == LOW) ? 1 : 0;  // Active LOW

  // Create JSON
  String jsonData = "{";
  jsonData += "\"id\":\"P1\",";
  jsonData += "\"temperature\":" + String(temperature, 1) + ",";
  jsonData += "\"pressure\":" + String(pressure, 1) + ",";
  jsonData += "\"altitude\":" + String(altitude, 1) + ",";
  jsonData += "\"battery\":80,";
  jsonData += "\"alert\":" + String(alertFlag);
  jsonData += "}";

  // Send JSON via LoRa
  LoRa.beginPacket();
  LoRa.print(jsonData);
  LoRa.endPacket();

  Serial.println("Sent: " + jsonData);

  delay(5000);
}
