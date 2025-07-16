#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include <SPI.h>

#define SD_CS 5  // Adjust if your SD_CS pin is different

WebServer server(80);

// Wi-Fi Access Point credentials
const char* ssid = "ESP32-MAP";
const char* password = "12345678";

// Serve any file requested (index.html, leaflet files, tiles)
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

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Start Wi-Fi Access Point
  WiFi.softAP(ssid, password);
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Initialize SD card
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card initialization failed!");
    return;
  }
  Serial.println("SD Card initialized.");

  // Handle all file requests
  server.onNotFound(handleFileRequest);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
