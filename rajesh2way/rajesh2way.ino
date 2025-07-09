#include <WiFi.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// LoRa pins
#define SS   5
#define RST  14
#define DIO0 2

// === CHANGE THESE FOR EACH NODE ===
const char* ssid = "trekker2";      // Use "trekker2" for Node2
const char* password = "password2"; // Use "password2" for Node2
const char* localNode = "Node2";    // Use "Node2" for Node2
const char* remoteNode = "Node1";   // Use "Node1" for Node2
const unsigned long sendInterval = 12000; // Use 12000 for trekker2

Adafruit_BMP085 bmp;
AsyncWebServer server(80);

// Local sensor data
float localTemp = 0, localPres = 0;
int localCount = 0;

// Remote node data
float remoteTemp = 0, remotePres = 0;
int remoteCount = 0;
unsigned long lastRemoteUpdate = 0;

unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!bmp.begin()) {
    Serial.println("Could not find BMP180 sensor!");
    while (1);
  }

  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed!");
    while (1);
  }
  LoRa.setSyncWord(0xF3);

  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head><title>LoRa Trekker Dashboard</title>"
      "<meta name='viewport' content='width=device-width, initial-scale=1'>"
      "<style>body{font-family:sans-serif;text-align:center;padding:20px;}h2{color:#333;}table{margin:auto;border-collapse:collapse;}td,th{padding:8px 16px;border:1px solid #ccc;}</style>"
      "</head><body><h2>LoRa Trekker Dashboard (" + String(localNode) + ")</h2>"
      "<table><tr><th>Node</th><th>Temperature (°C)</th><th>Pressure (hPa)</th><th>Count</th></tr>"
      "<tr><td>" + String(localNode) + "</td><td>" + String(localTemp,2) + "</td><td>" + String(localPres,2) + "</td><td>" + String(localCount) + "</td></tr>"
      "<tr><td>" + String(remoteNode) + "</td><td>" + String(remoteTemp,2) + "</td><td>" + String(remotePres,2) + "</td><td>" + String(remoteCount) + "</td></tr></table>"
      "<p>Last update from " + String(remoteNode) + ": " + String((millis()-lastRemoteUpdate)/1000) + "s ago</p>"
      "</body></html>";
    request->send(200, "text/html", html);
  });

  server.begin();
}

void loop() {
  // 1. Always listen for incoming LoRa packets
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) received += (char)LoRa.read();
    Serial.println("Received: " + received);

    // Parse remote data if from the other node
    int nodeIdx = received.indexOf("Node:");
    if (nodeIdx >= 0) {
      int nodeEnd = received.indexOf(",", nodeIdx);
      String nodeName = received.substring(nodeIdx + 5, nodeEnd);
      if (nodeName == remoteNode) {
        int t1 = received.indexOf("Temp:") + 5;
        int t2 = received.indexOf("C", t1);
        int p1 = received.indexOf("Pressure:") + 9;
        int p2 = received.indexOf("hPa", p1);
        int c1 = received.indexOf("Count:") + 6;

        if (t1 > 4 && t2 > t1 && p1 > 8 && p2 > p1 && c1 > 5) {
          remoteTemp = received.substring(t1, t2).toFloat();
          remotePres = received.substring(p1, p2).toFloat();
          remoteCount = received.substring(c1).toInt();
          lastRemoteUpdate = millis();
        }
      }
    }
  }

  // 2. Send data at regular intervals (non-blocking)
  if (millis() - lastSend >= sendInterval) {
    localTemp = bmp.readTemperature();
    localPres = bmp.readPressure() / 100.0;

    String payload = "Node:" + String(localNode) +
                     ",Temp:" + String(localTemp,2) + "C" +
                     ",Pressure:" + String(localPres,2) + "hPa" +
                     ",Count:" + String(localCount);
    LoRa.beginPacket();
    LoRa.print(payload);
    LoRa.endPacket();
    Serial.println("Sent: " + payload);
    localCount++;
    lastSend = millis();

    delay(100); // Short delay to allow LoRa module to switch back to RX
  }

  // No blocking delay here! Node is always ready to receive.
}
