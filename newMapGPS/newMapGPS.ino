#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <WiFi.h>
#include <WebServer.h>

// Replace with your WiFi credentials
const char* ssid = "Big b 003";
const char* password = "7411809576";

TinyGPS gps;
SoftwareSerial ss(13, 12); // RX, TX (connect GPS TX to ESP32 RX13, GPS RX to ESP32 TX12)
WebServer server(80);

float flat, flon;
unsigned long age;

void setup() {
  Serial.begin(115200);
  ss.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  // Endpoint to provide GPS coordinates
  server.on("/loc", []() {
    gps.f_get_position(&flat, &flon, &age);
    String text = String(flat, 6) + "," + String(flon, 6);
    server.send(200, "text/plain", text);
    smartdelay(500);
  });

  // Home page (replace with your HTML/JS for Google Maps)
  server.on("/", []() {
    server.send(200, "text/html", "<html><body><h1>ESP32 GPS Map</h1></body></html>");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

// Helper to keep GPS data updated
void smartdelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}
