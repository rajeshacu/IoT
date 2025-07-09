#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "CoE-IOT";
const char* password = "12345678";

// Web server on port 80
WebServer server(80);

// Dummy GPS coordinates (you can replace with real GPS reading)
struct GPSData {
  float latitude;
  float longitude;
  float altitude;
  int satellites;
  bool valid;
};

GPSData currentGPS;

// Simulate GPS movement
float base_lat = 40.7128;  // New York City
float base_lon = -74.0060;
float lat_offset = 0.0;
float lon_offset = 0.0;

// Function declarations
void handleRoot();
void handleGPS();
void handleCSS();
void simulateGPSMovement();

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Connected to WiFi. IP: ");
  Serial.println(WiFi.localIP());
  
  // Initialize GPS data
  currentGPS.latitude = base_lat;
  currentGPS.longitude = base_lon;
  currentGPS.altitude = 10.0;
  currentGPS.satellites = 8;
  currentGPS.valid = true;
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/gps", handleGPS);
  server.on("/style.css", handleCSS);
  
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
  
  // Simulate GPS movement every 3 seconds
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 3000) {
    simulateGPSMovement();
    lastUpdate = millis();
  }
}

void simulateGPSMovement() {
  // Add small random movement
  lat_offset += (random(-10, 11) / 10000.0);  // ±0.001 degrees
  lon_offset += (random(-10, 11) / 10000.0);
  
  // Keep within reasonable bounds
  if (abs(lat_offset) > 0.01) lat_offset = 0.0;
  if (abs(lon_offset) > 0.01) lon_offset = 0.0;
  
  currentGPS.latitude = base_lat + lat_offset;
  currentGPS.longitude = base_lon + lon_offset;
  currentGPS.altitude = 10.0 + random(-5, 6);
  currentGPS.satellites = random(6, 12);
  
  Serial.printf("GPS: %.6f, %.6f\n", currentGPS.latitude, currentGPS.longitude);
}

void handleRoot() {
  String html = "<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"    <meta charset=\"UTF-8\">"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"    <title>GPS Tracker</title>"
"    <link rel=\"stylesheet\" href=\"/style.css\">"
"    <link rel=\"stylesheet\" href=\"https://unpkg.com/leaflet@1.7.1/dist/leaflet.css\" />"
"</head>"
"<body>"
"    <div class=\"container\">"
"        <h1>🛰️ GPS Tracker</h1>"
"        <div class=\"status-panel\">"
"            <div class=\"status-item\">"
"                <span class=\"label\">Status:</span>"
"                <span id=\"status\" class=\"status-connected\">Connected</span>"
"            </div>"
"            <div class=\"status-item\">"
"                <span class=\"label\">Satellites:</span>"
"                <span id=\"satellites\">--</span>"
"            </div>"
"            <div class=\"status-item\">"
"                <span class=\"label\">Altitude:</span>"
"                <span id=\"altitude\">--</span>"
"            </div>"
"        </div>"
"        <div class=\"coordinates\">"
"            <div class=\"coord-item\">"
"                <span class=\"label\">Latitude:</span>"
"                <span id=\"latitude\">--</span>"
"            </div>"
"            <div class=\"coord-item\">"
"                <span class=\"label\">Longitude:</span>"
"                <span id=\"longitude\">--</span>"
"            </div>"
"        </div>"
"        <div id=\"map\"></div>"
"        <div class=\"controls\">"
"            <button id=\"centerBtn\" class=\"btn\">📍 Center Map</button>"
"            <button id=\"toggleBtn\" class=\"btn\">⏸️ Pause</button>"
"        </div>"
"    </div>"
"    <script src=\"https://unpkg.com/leaflet@1.7.1/dist/leaflet.js\"></script>"
"    <script>"
"        var map, marker, circle;"
"        var isTracking = true;"
"        var currentPosition = null;"
"        "
"        function initMap() {"
"            map = L.map('map').setView([40.7128, -74.0060], 15);"
"            L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {"
"                attribution: '© OpenStreetMap contributors'"
"            }).addTo(map);"
"            marker = L.marker([40.7128, -74.0060]).addTo(map);"
"            circle = L.circle([40.7128, -74.0060], {"
"                color: 'blue',"
"                fillColor: '#30f',"
"                fillOpacity: 0.2,"
"                radius: 50"
"            }).addTo(map);"
"        }"
"        "
"        function fetchGPSData() {"
"            fetch('/gps')"
"                .then(function(response) { return response.json(); })"
"                .then(function(data) {"
"                    if (data.valid) {"
"                        currentPosition = [data.latitude, data.longitude];"
"                        updateDisplay(data);"
"                        if (isTracking) {"
"                            updateMap(data);"
"                        }"
"                    }"
"                })"
"                .catch(function(error) {"
"                    console.error('Error fetching GPS data:', error);"
"                    document.getElementById('status').textContent = 'Error';"
"                    document.getElementById('status').className = 'status-error';"
"                });"
"        }"
"        "
"        function updateDisplay(data) {"
"            document.getElementById('latitude').textContent = data.latitude.toFixed(6);"
"            document.getElementById('longitude').textContent = data.longitude.toFixed(6);"
"            document.getElementById('altitude').textContent = data.altitude.toFixed(1) + 'm';"
"            document.getElementById('satellites').textContent = data.satellites;"
"            document.getElementById('status').textContent = 'Connected';"
"            document.getElementById('status').className = 'status-connected';"
"        }"
"        "
"        function updateMap(data) {"
"            var pos = [data.latitude, data.longitude];"
"            marker.setLatLng(pos);"
"            circle.setLatLng(pos);"
"            if (isTracking) {"
"                map.setView(pos, map.getZoom());"
"            }"
"        }"
"        "
"        document.addEventListener('DOMContentLoaded', function() {"
"            document.getElementById('centerBtn').addEventListener('click', function() {"
"                if (currentPosition) {"
"                    map.setView(currentPosition, 15);"
"                }"
"            });"
"            "
"            document.getElementById('toggleBtn').addEventListener('click', function() {"
"                isTracking = !isTracking;"
"                this.textContent = isTracking ? '⏸️ Pause' : '▶️ Resume';"
"                this.className = isTracking ? 'btn' : 'btn btn-paused';"
"            });"
"            "
"            initMap();"
"            fetchGPSData();"
"            setInterval(fetchGPSData, 2000);"
"        });"
"    </script>"
"</body>"
"</html>";
  
  server.send(200, "text/html", html);
}

void handleGPS() {
  StaticJsonDocument<200> doc;
  doc["latitude"] = currentGPS.latitude;
  doc["longitude"] = currentGPS.longitude;
  doc["altitude"] = currentGPS.altitude;
  doc["satellites"] = currentGPS.satellites;
  doc["valid"] = currentGPS.valid;
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
}

void handleCSS() {
  String css = "* {"
"    margin: 0;"
"    padding: 0;"
"    box-sizing: border-box;"
"}"
"body {"
"    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;"
"    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
"    min-height: 100vh;"
"    padding: 10px;"
"}"
".container {"
"    max-width: 500px;"
"    margin: 0 auto;"
"    background: white;"
"    border-radius: 20px;"
"    box-shadow: 0 20px 40px rgba(0,0,0,0.1);"
"    overflow: hidden;"
"}"
"h1 {"
"    background: linear-gradient(45deg, #667eea, #764ba2);"
"    color: white;"
"    padding: 20px;"
"    text-align: center;"
"    font-size: 24px;"
"    font-weight: 600;"
"}"
".status-panel {"
"    display: flex;"
"    justify-content: space-between;"
"    padding: 15px 20px;"
"    background: #f8f9fa;"
"    border-bottom: 1px solid #e9ecef;"
"}"
".status-item {"
"    text-align: center;"
"    flex: 1;"
"}"
".label {"
"    display: block;"
"    font-size: 12px;"
"    color: #6c757d;"
"    margin-bottom: 5px;"
"    font-weight: 500;"
"}"
".status-connected {"
"    color: #28a745;"
"    font-weight: 600;"
"}"
".status-error {"
"    color: #dc3545;"
"    font-weight: 600;"
"}"
".coordinates {"
"    padding: 20px;"
"}"
".coord-item {"
"    display: flex;"
"    justify-content: space-between;"
"    margin-bottom: 15px;"
"    padding: 10px;"
"    background: #f8f9fa;"
"    border-radius: 8px;"
"}"
".coord-item .label {"
"    font-weight: 600;"
"    color: #495057;"
"}"
"#map {"
"    height: 300px;"
"    margin: 0 20px 20px 20px;"
"    border-radius: 10px;"
"    box-shadow: 0 4px 8px rgba(0,0,0,0.1);"
"}"
".controls {"
"    display: flex;"
"    gap: 10px;"
"    padding: 20px;"
"    padding-top: 0;"
"}"
".btn {"
"    flex: 1;"
"    padding: 12px;"
"    border: none;"
"    border-radius: 8px;"
"    background: #667eea;"
"    color: white;"
"    font-size: 14px;"
"    font-weight: 600;"
"    cursor: pointer;"
"    transition: all 0.3s ease;"
"}"
".btn:hover {"
"    background: #5a6fd8;"
"    transform: translateY(-2px);"
"}"
".btn-paused {"
"    background: #ffc107;"
"    color: #212529;"
"}"
".btn-paused:hover {"
"    background: #ffca28;"
"}"
"@media (max-width: 480px) {"
"    .container {"
"        margin: 0;"
"        border-radius: 0;"
"        min-height: 100vh;"
"    }"
"    .status-panel {"
"        flex-direction: column;"
"        gap: 10px;"
"    }"
"    .status-item {"
"        display: flex;"
"        justify-content: space-between;"
"        align-items: center;"
"    }"
"    .status-item .label {"
"        margin-bottom: 0;"
"    }"
"    #map {"
"        height: 250px;"
"        margin: 0 10px 20px 10px;"
"    }"
"}";
  
  server.send(200, "text/css", css);
}