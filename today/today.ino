#include <WiFi.h>
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>

// WiFi credentials
const char *ssid = "Your_SSID";
const char *password = "Your_Password";

// Web server on port 80
WiFiServer server(80);

// LoRa pins
#define LORA_SCK 18
#define LORA_MISO 19
#define LORA_MOSI 23
#define LORA_SS 5
#define LORA_RST 14
#define LORA_DIO0 2

// Global sensor values
float temp = 0.0;
int32_t pressure = 0;
float altitude = 0.0;

void setup() {
  Serial.begin(115200);

  // Init LoRa
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("❌ LoRa init failed");
    while (1);
  }
  Serial.println("✅ LoRa Receiver ready");

  // Init WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("📶 Connected. IP: ");
  Serial.println(WiFi.localIP());

  server.begin(); // Start web server
}

void loop() {
  // Handle LoRa reception
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String jsonStr = "";
    while (LoRa.available()) {
      jsonStr += (char)LoRa.read();
    }

    Serial.println("📥 Received: " + jsonStr);

    // Parse JSON
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    if (!error) {
      temp = doc["temperature"];
      pressure = doc["pressure"];
      altitude = doc["altitude"];
    } else {
      Serial.println("❌ JSON Parse error");
    }
  }

  // Handle Web requests
  WiFiClient client = server.available();
  if (client) {
    Serial.println("🌐 New Web Client");

    // Wait for client request
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        currentLine += c;
        if (c == '\n' && currentLine.length() == 1) {
          // HTTP Header - HTML Page
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html\r\n");

          client.println("<!DOCTYPE html><html><head><title>LoRa Sensor Dashboard</title>");
          client.println("<meta http-equiv='refresh' content='5'>");
          client.println("<style>body{font-family:sans-serif;background:#222;color:#0f0;}</style></head>");
          client.println("<body><h1>📶 LoRa Sensor Live Dashboard</h1>");
          client.println("<p>🌡️ Temperature: " + String(temp) + " °C</p>");
          client.println("<p>🔵 Pressure: " + String(pressure / 100.0) + " hPa</p>");
          client.println("<p>⛰️ Altitude: " + String(altitude) + " m</p>");
          client.println("<p>🟢 Data auto-refreshes every 5 seconds</p>");
          client.println("</body></html>");
          break;
        }
      }
    }
    delay(1);
    client.stop();
    Serial.println("✖️ Client disconnected");
  }
}
