#include <WiFi.h> 
#include <WebServer.h>
#include <ArduinoJson.h>

// Create an Access Point (No WiFi needed)
const char* ap_ssid = "ESP32-GPS-Tracker";
const char* ap_password = "tracker123";  // Set to "" for open network

WebServer server(80);  // HTTP server on port 80

// GPS Data Structure
struct GPSData {
  float latitude;
  float longitude;
  float altitude;
  int satellites;
  bool valid;
};

GPSData currentGPS;

// Base coordinates (New York City)
float base_lat = 40.7128;
float base_lon = -74.0060;
float lat_offset = 0.0;
float lon_offset = 0.0;

void setup() {
  Serial.begin(115200);

  // Start Access Point
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("\nAccess Point Started!");
  Serial.print("SSID: ");
  Serial.println(ap_ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  // Initialize GPS with default values
  currentGPS.latitude = base_lat;
  currentGPS.longitude = base_lon;
  currentGPS.altitude = 10.0;
  currentGPS.satellites = 8;
  currentGPS.valid = true;

  // Define server routes
  server.on("/", handleRoot);
  server.on("/gps", handleGPS);
  server.on("/style.css", handleCSS);

  server.begin();
  Serial.println("HTTP Server Started");
}

void loop() {
  server.handleClient();  // Handle web requests

  // Simulate GPS movement every 3 seconds
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 3000) {
    simulateGPSMovement();
    lastUpdate = millis();
  }
}

// Simulate GPS movement with small random changes
void simulateGPSMovement() {
  lat_offset += (random(-10, 11) / 10000.0);  // Small random steps
  lon_offset += (random(-10, 11) / 10000.0);

  // Reset if drift is too large
  if (abs(lat_offset) > 0.01) lat_offset = 0.0;
  if (abs(lon_offset) > 0.01) lon_offset = 0.0;

  currentGPS.latitude = base_lat + lat_offset;
  currentGPS.longitude = base_lon + lon_offset;
  currentGPS.altitude = 10.0 + random(-5, 6);
  currentGPS.satellites = random(6, 12);

  Serial.printf("Simulated GPS: %.6f, %.6f\n", currentGPS.latitude, currentGPS.longitude);
}

// Serve the main HTML page
void handleRoot() {
  String html = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 GPS Tracker (Offline)</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <h1>📍 ESP32 GPS Tracker (Offline)</h1>
        <div class="status-panel">
            <div class="status-item">
                <span class="label">Status:</span>
                <span id="status" class="status-connected">Connected</span>
            </div>
            <div class="status-item">
                <span class="label">Satellites:</span>
                <span id="satellites">--</span>
            </div>
            <div class="status-item">
                <span class="label">Altitude:</span>
                <span id="altitude">--</span>
            </div>
        </div>
        <div class="coordinates">
            <div class="coord-item">
                <span class="label">Latitude:</span>
                <span id="latitude">--</span>
            </div>
            <div class="coord-item">
                <span class="label">Longitude:</span>
                <span id="longitude">--</span>
            </div>
        </div>
        <div class="map-container">
            <canvas id="map" width="400" height="300"></canvas>
        </div>
        <div class="controls">
            <button id="centerBtn" class="btn">📍 Center</button>
            <button id="toggleBtn" class="btn">⏸️ Pause</button>
            <button id="zoomInBtn" class="btn">🔍+</button>
            <button id="zoomOutBtn" class="btn">🔍-</button>
        </div>
        <div class="map-info">
            <div class="info-item">
                <span class="label">Zoom:</span>
                <span id="zoom-level">1x</span>
            </div>
            <div class="info-item">
                <span class="label">Speed:</span>
                <span id="speed">0 km/h</span>
            </div>
        </div>
    </div>
    <script>
        var canvas, ctx;
        var isTracking = true;
        var currentPosition = null;
        var lastPosition = null;
        var zoom = 1;
        var centerX = 200, centerY = 150;
        var speed = 0;
        var lastUpdateTime = 0;

        function initMap() {
            canvas = document.getElementById('map');
            ctx = canvas.getContext('2d');
            drawMap();
        }

        function drawMap() {
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            
            // Draw background (simulating sky/water)
            ctx.fillStyle = '#e6f3ff';
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            
            // Draw grid lines (simulating a map)
            ctx.strokeStyle = '#d0d0d0';
            for (var i = 0; i <= canvas.width; i += 20) {
                ctx.beginPath();
                ctx.moveTo(i, 0);
                ctx.lineTo(i, canvas.height);
                ctx.stroke();
            }
            for (var i = 0; i <= canvas.height; i += 20) {
                ctx.beginPath();
                ctx.moveTo(0, i);
                ctx.lineTo(canvas.width, i);
                ctx.stroke();
            }
            
            // Draw current position
            if (currentPosition) {
                var x = centerX + (currentPosition.longitude - currentPosition.baseLon) * 100000 * zoom;
                var y = centerY - (currentPosition.latitude - currentPosition.baseLat) * 100000 * zoom;
                
                // Draw position marker
                ctx.fillStyle = '#ff0000';
                ctx.beginPath();
                ctx.arc(x, y, 5 * zoom, 0, 2 * Math.PI);
                ctx.fill();
                
                // Draw direction arrow if moving
                if (speed > 0) {
                    ctx.strokeStyle = '#ff0000';
                    ctx.beginPath();
                    ctx.moveTo(x, y);
                    ctx.lineTo(x, y - 15 * zoom);
                    ctx.lineTo(x - 5 * zoom, y - 10 * zoom);
                    ctx.moveTo(x, y - 15 * zoom);
                    ctx.lineTo(x + 5 * zoom, y - 10 * zoom);
                    ctx.stroke();
                }
            }
            
            // Draw compass
            ctx.fillStyle = '#333';
            ctx.font = '12px Arial';
            ctx.fillText('N', canvas.width - 20, 15);
            ctx.fillText('S', canvas.width - 20, canvas.height - 5);
            ctx.fillText('E', canvas.width - 10, canvas.height / 2);
            ctx.fillText('W', canvas.width - 30, canvas.height / 2);
        }

        function fetchGPSData() {
            fetch('/gps')
                .then(response => response.json())
                .then(data => {
                    if (data.valid) {
                        calculateSpeed(data);
                        currentPosition = {
                            latitude: data.latitude,
                            longitude: data.longitude,
                            baseLat: 40.7128,
                            baseLon: -74.0060
                        };
                        updateDisplay(data);
                        if (isTracking) drawMap();
                    }
                })
                .catch(error => {
                    console.error('GPS Error:', error);
                    document.getElementById('status').textContent = 'Error';
                    document.getElementById('status').className = 'status-error';
                });
        }

        function calculateSpeed(data) {
            var currentTime = Date.now();
            if (lastPosition && lastUpdateTime > 0) {
                var distance = getDistance(lastPosition.latitude, lastPosition.longitude, data.latitude, data.longitude);
                var timeElapsed = (currentTime - lastUpdateTime) / 1000; // seconds
                speed = (distance / timeElapsed) * 3.6; // km/h
            }
            lastPosition = {latitude: data.latitude, longitude: data.longitude};
            lastUpdateTime = currentTime;
        }

        function getDistance(lat1, lon1, lat2, lon2) {
            var R = 6371000; // Earth radius in meters
            var dLat = (lat2 - lat1) * Math.PI / 180;
            var dLon = (lon2 - lon1) * Math.PI / 180;
            var a = Math.sin(dLat/2) * Math.sin(dLat/2) + Math.cos(lat1 * Math.PI / 180) * Math.cos(lat2 * Math.PI / 180) * Math.sin(dLon/2) * Math.sin(dLon/2);
            var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
            return R * c;
        }

        function updateDisplay(data) {
            document.getElementById('latitude').textContent = data.latitude.toFixed(6);
            document.getElementById('longitude').textContent = data.longitude.toFixed(6);
            document.getElementById('altitude').textContent = data.altitude.toFixed(1) + 'm';
            document.getElementById('satellites').textContent = data.satellites;
            document.getElementById('zoom-level').textContent = zoom.toFixed(1) + 'x';
            document.getElementById('speed').textContent = speed.toFixed(1) + ' km/h';
        }

        // Event Listeners
        document.addEventListener('DOMContentLoaded', () => {
            initMap();
            fetchGPSData();
            setInterval(fetchGPSData, 2000);

            document.getElementById('centerBtn').addEventListener('click', () => {
                centerX = 200;
                centerY = 150;
                drawMap();
            });

            document.getElementById('toggleBtn').addEventListener('click', () => {
                isTracking = !isTracking;
                this.textContent = isTracking ? '⏸️ Pause' : '▶️ Resume';
            });

            document.getElementById('zoomInBtn').addEventListener('click', () => {
                zoom = Math.min(zoom * 1.5, 10);
                drawMap();
                document.getElementById('zoom-level').textContent = zoom.toFixed(1) + 'x';
            });

            document.getElementById('zoomOutBtn').addEventListener('click', () => {
                zoom = Math.max(zoom / 1.5, 0.1);
                drawMap();
                document.getElementById('zoom-level').textContent = zoom.toFixed(1) + 'x';
            });

            // Map dragging (panning)
            let isDragging = false;
            let lastX, lastY;

            canvas.addEventListener('mousedown', (e) => {
                isDragging = true;
                lastX = e.clientX;
                lastY = e.clientY;
            });

            canvas.addEventListener('mousemove', (e) => {
                if (isDragging) {
                    centerX += e.clientX - lastX;
                    centerY += e.clientY - lastY;
                    lastX = e.clientX;
                    lastY = e.clientY;
                    drawMap();
                }
            });

            canvas.addEventListener('mouseup', () => {
                isDragging = false;
            });

            // Touch support for mobile
            canvas.addEventListener('touchstart', (e) => {
                e.preventDefault();
                const touch = e.touches[0];
                isDragging = true;
                lastX = touch.clientX;
                lastY = touch.clientY;
            });

            canvas.addEventListener('touchmove', (e) => {
                e.preventDefault();
                if (isDragging) {
                    const touch = e.touches[0];
                    centerX += touch.clientX - lastX;
                    centerY += touch.clientY - lastY;
                    lastX = touch.clientX;
                    lastY = touch.clientY;
                    drawMap();
                }
            });

            canvas.addEventListener('touchend', () => {
                isDragging = false;
            });
        });
    </script>
</body>
</html>
)=====";

  server.send(200, "text/html", html);
}

// Serve GPS data as JSON
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

// Serve CSS styles
void handleCSS() {
  String css = R"=====(
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}
body {
    font-family: Arial, sans-serif;
    background: linear-gradient(135deg, #667eea, #764ba2);
    min-height: 100vh;
    padding: 10px;
}
.container {
    max-width: 500px;
    margin: 0 auto;
    background: white;
    border-radius: 20px;
    box-shadow: 0 10px 20px rgba(0,0,0,0.1);
    overflow: hidden;
}
h1 {
    background: linear-gradient(45deg, #667eea, #764ba2);
    color: white;
    padding: 15px;
    text-align: center;
    font-size: 20px;
}
.status-panel {
    display: flex;
    justify-content: space-between;
    padding: 10px 15px;
    background: #f8f9fa;
    border-bottom: 1px solid #e9ecef;
}
.status-item {
    text-align: center;
    flex: 1;
}
.label {
    display: block;
    font-size: 12px;
    color: #666;
    margin-bottom: 3px;
}
.status-connected {
    color: #28a745;
    font-weight: bold;
}
.status-error {
    color: #dc3545;
    font-weight: bold;
}
.coordinates {
    padding: 15px;
}
.coord-item {
    display: flex;
    justify-content: space-between;
    margin-bottom: 10px;
    padding: 8px;
    background: #f8f9fa;
    border-radius: 8px;
}
.map-container {
    margin: 10px;
    border-radius: 10px;
    overflow: hidden;
    box-shadow: 0 4px 8px rgba(0,0,0,0.1);
}
#map {
    display: block;
    width: 100%;
    height: 250px;
    background: #e6f3ff;
    cursor: move;
}
.controls {
    display: grid;
    grid-template-columns: repeat(4, 1fr);
    gap: 8px;
    padding: 10px;
}
.btn {
    padding: 10px;
    border: none;
    border-radius: 8px;
    background: #667eea;
    color: white;
    font-size: 12px;
    cursor: pointer;
    transition: background 0.3s;
}
.btn:hover {
    background: #5a6fd8;
}
.map-info {
    display: flex;
    justify-content: space-between;
    padding: 10px;
    background: #f8f9fa;
    border-top: 1px solid #e9ecef;
}
@media (max-width: 480px) {
    .container {
        margin: 0;
        border-radius: 0;
    }
    #map {
        height: 200px;
    }
    .controls {
        grid-template-columns: repeat(2, 1fr);
    }
}
)=====";

  server.send(200, "text/css", css);
}