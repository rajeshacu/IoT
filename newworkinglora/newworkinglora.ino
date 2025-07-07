#include <WiFi.h>
#include <SPI.h>
#include <LoRa.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// LoRa pins
#define SS   5
#define RST  14
#define DIO0 2

const char* ssid = "LoRaReceiverAP";
const char* password = "12345678";

AsyncWebServer server(80);

String lastMessage = "No data";
int lastRSSI = 0;

void notifyClients(String msg) {
  // For simplicity, no WebSocket here; page polls /data endpoint
}

void setup() {
  Serial.begin(115200);

  // Start LoRa
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(433E6)) {  // Match sender frequency
    Serial.println("LoRa init failed!");
    while (1);
  }
  LoRa.setSyncWord(0xF3);

  Serial.println("LoRa Receiver started");

  // Start WiFi AP
  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"rawliteral(
      <!DOCTYPE html><html><head><title>LoRa BMP180 Receiver</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>body{font-family:sans-serif;text-align:center;padding:20px;}
      h2{color:#333;} p{font-size:1.2em;}
      </style></head><body>
      <h2>LoRa BMP180 Receiver</h2>
      <p><b>Last Message:</b> <span id="msg">No data</span></p>
      <p><b>RSSI:</b> <span id="rssi">0</span></p>
      <script>
        setInterval(() => {
          fetch('/data').then(res => res.json()).then(data => {
            document.getElementById('msg').textContent = data.msg;
            document.getElementById('rssi').textContent = data.rssi;
          });
        }, 2000);
      </script>
      </body></html>
    )rawliteral";
    request->send(200, "text/html", html);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"msg\":\"" + lastMessage + "\",\"rssi\":" + String(lastRSSI) + "}";
    request->send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }
    lastMessage = received;
    lastRSSI = LoRa.packetRssi();

    Serial.print("Received: ");
    Serial.print(lastMessage);
    Serial.print(" | RSSI: ");
    Serial.println(lastRSSI);
  }
}
