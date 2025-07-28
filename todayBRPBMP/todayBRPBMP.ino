#include <WiFi.h>
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>

// LoRa pins
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23
#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  2

// AP credentials (change as needed)
const char* ssid = "ESP32-LoRa-AP";
const char* password = "12345678"; // Min 8 chars

WiFiServer server(80);

// Data storage
float temperature = 0.0;
long pressure = 0;
float altitude = 0.0;

void setup() {
  Serial.begin(115200);

  // Start LoRa
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check wiring!");
    while (1);
  }
  Serial.println("LoRa Receiver Ready.");

  // Set up WiFi AP
  WiFi.softAP(ssid, password);
  delay(100);  // Allow AP setup time
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  server.begin();
}

void loop() {
  // LoRa: receive and parse JSON
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }
    Serial.println("LoRa packet: " + incoming);

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, incoming);
    if (!error) {
      if (doc.containsKey("temperature")) temperature = doc["temperature"];
      if (doc.containsKey("pressure"))    pressure = doc["pressure"];
      if (doc.containsKey("altitude"))    altitude = doc["altitude"];
    } else {
      Serial.println("Invalid JSON received");
    }
  }

  // Web Server: serve page to connected clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client Connected!");
    // Wait for end of GET request
    String req = "";
    while (client.connected() && client.available() == 0)
      delay(1);
    while (client.available()) {
      char c = client.read();
      req += c;
      // (optional: break when "\r\n\r\n" received)
    }

    // HTTP header
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html\r\n");

    // Web page (auto-refreshes every 5 seconds)
    client.println("<!DOCTYPE html><html><head><title>LoRa Receiver Dashboard</title>");
    client.println("<meta http-equiv='refresh' content='5'/>");
    client.println("<style>body{background:#222;color:#0f0;font-family:sans-serif;padding:2em;}");
    client.println("div{font-size:2em;margin-bottom:1em;}</style></head><body>");
    client.println("<h2>ESP32 LoRa Sensor Dashboard (Offline WiFi AP)</h2>");
    client.print("<div> Temperature: "); client.print(temperature); client.println(" &deg;C</div>");
    client.print("<div>Pressure: "); client.print(pressure / 100.0); client.println(" hPa</div>");
    client.print("<div> Altitude: "); client.print(altitude); client.println(" m</div>");
    client.println("<p>(Auto-refresh every 5s. Connect to WiFi SSID: <b>ESP32-LoRa-AP</b>)</p>");
    client.println("</body></html>");

    client.stop(); // Disconnect client
    Serial.println("Client disconnected.");
  }
}
