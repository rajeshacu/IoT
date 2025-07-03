#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Replace with your desired SSID and password
const char* ssid = "ESP32-Tracker";
const char* password = "12345678";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncEventSource events("/events"); // SSE endpoint

// Variables to hold the latest sensor readings
String gpsLocation = "N/A";
String batteryPercent = "N/A";
String temperature = "N/A";
String pressure = "N/A";

// Example: This function should be called when a new LoRa packet is received
void updateSensorReadings(String newGPS, String newBatt, String newTemp, String newPress) {
  gpsLocation = newGPS;
  batteryPercent = newBatt;
  temperature = newTemp;
  pressure = newPress;

  // Send new readings to all connected clients via SSE
  String json = "{\"gps\":\"" + gpsLocation + "\",\"battery\":\"" + batteryPercent + "\",\"temperature\":\"" + temperature + "\",\"pressure\":\"" + pressure + "\"}";
  events.send(json.c_str(), "new_readings", millis());
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP32 Tracker Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; margin: 0; }
    .card { background: #f4f4f4; padding: 20px; margin: 20px auto; width: 300px; border-radius: 10px; }
    h2 { color: #333; }
    .reading { font-size: 1.4em; color: #0078D7; }
  </style>
</head>
<body>
  <h2>ESP32 LoRa Tracker</h2>
  <div class="card">
    <b>GPS Location:</b>
    <div class="reading" id="gps">N/A</div>
    <b>Battery:</b>
    <div class="reading" id="battery">N/A</div>
    <b>Temperature:</b>
    <div class="reading" id="temperature">N/A</div>
    <b>Pressure:</b>
    <div class="reading" id="pressure">N/A</div>
  </div>
  <script>
    if (!!window.EventSource) {
      var source = new EventSource('/events');
      source.addEventListener('new_readings', function(e) {
        var data = JSON.parse(e.data);
        document.getElementById('gps').innerHTML = data.gps;
        document.getElementById('battery').innerHTML = data.battery + " %";
        document.getElementById('temperature').innerHTML = data.temperature + " °C";
        document.getElementById('pressure').innerHTML = data.pressure + " hPa";
      }, false);
    }
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  // Set up Wi-Fi Access Point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Serve the main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // SSE endpoint
  server.addHandler(&events);

  // Start server
  server.begin();
}

void loop() {
  // Here, you would receive LoRa packets and parse them.
  // For demonstration, we'll simulate new data every 10 seconds:
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 10000) {
    lastUpdate = millis();
    // Replace the following lines with actual LoRa data parsing:
    updateSensorReadings("28.6139N, 77.2090E", "82", "29.3", "1002.5");
  }
}
