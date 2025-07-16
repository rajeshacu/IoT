#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

// Set your desired WiFi credentials for AP mode
const char* ssid = "Trekker_AP";
const char* password = "12345678";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // Start WiFi in Access Point mode for offline use
  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  // Serve index.html at root
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // Serve all static files (CSS, JS, images)
  server.serveStatic("/", SPIFFS, "/");

  server.begin();
}

void loop() {
  // Nothing needed here
}
