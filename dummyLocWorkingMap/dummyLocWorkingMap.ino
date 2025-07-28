#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include <SPI.h>

#define SD_CS 5

WebServer server(80);

// Wi-Fi Access Point credentials
const char* ssid = "ESP32-MAP";
const char* password = "12345678";

// Dummy GPS position
float latitude = 12.9716;
float longitude = 77.5946;

void handleFileRequest() {
  String path = server.uri();
  if (path == "/") path = "/index.html";

  String contentType = "text/plain";
  if (path.endsWith(".html")) contentType = "text/html";
  else if (path.endsWith(".css")) contentType = "text/css";
  else if (path.endsWith(".js")) contentType = "application/javascript";
  else if (path.endsWith(".png")) contentType = "image/png";
  else if (path.endsWith(".jpg")) contentType = "image/jpeg";
  else if (path.endsWith(".svg")) contentType = "image/svg+xml";

  File file = SD.open(path.c_str());
  if (!file || file.isDirectory()) {
    server.send(404, "text/plain", "File Not Found\n");
    return;
  }

  server.streamFile(file, contentType);
  file.close();
}

// Endpoint for GPS data
void handleLocationRequest() {
  // Simulate small movement
  latitude += 0.0001;
  longitude += 0.0001;

  String json = "{ \"lat\": " + String(latitude, 6) + ", \"lng\": " + String(longitude, 6) + " }";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.softAP(ssid, password);
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());

  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card initialization failed!");
    return;
  }
  Serial.println("SD Card initialized.");

  // Web handlers
  server.on("/location", handleLocationRequest);
  server.onNotFound(handleFileRequest);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
