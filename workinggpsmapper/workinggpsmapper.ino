#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include <SPI.h>

#define SD_CS 5

WebServer server(80);

const char* ssid = "ESP32-MAP";
const char* password = "12345678";

// Starting dummy location (Bengaluru)
float latitude = 12.9716;
float longitude = 77.5946;

unsigned long lastUpdate = 0;

// Serve HTML, JS, Tiles
void handleFileRequest() {
  String path = server.uri();
  if (path == "/") path = "/index.html";

  String contentType = "text/plain";
  if (path.endsWith(".html")) contentType = "text/html";
  else if (path.endsWith(".css")) contentType = "text/css";
  else if (path.endsWith(".js")) contentType = "application/javascript";
  else if (path.endsWith(".png")) contentType = "image/png";

  File file = SD.open(path.c_str());
  if (!file || file.isDirectory()) {
    server.send(404, "text/plain", "File Not Found\n");
    return;
  }

  server.streamFile(file, contentType);
  file.close();
}

// Return dummy GPS as JSON
void handleLocation() {
  String json = "{ \"lat\": " + String(latitude, 6) + ", \"lng\": " + String(longitude, 6) + " }";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  if (!SD.begin(SD_CS)) {
    Serial.println("SD card init failed!");
    return;
  }
  Serial.println("SD card initialized.");

  server.onNotFound(handleFileRequest);
  server.on("/location", handleLocation);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  // Simulate movement every 3 seconds
  if (millis() - lastUpdate > 3000) {
    latitude += 0.0001;   // Simulate small movement north
    longitude += 0.0001;  // Simulate small movement east
    lastUpdate = millis();

    Serial.printf("New dummy location: %.6f, %.6f\n", latitude, longitude);
  }
}
