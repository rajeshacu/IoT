#include <WiFi.h>
#include <WebServer.h>

#define BUTTON_PIN 4
bool alertTriggered = false;

WebServer server(80);

// Auto-refreshing HTML with a screen button
String getHtml() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP32 Emergency</title>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<style>body { font-family: Arial; text-align: center; padding: 20px; }";
  html += ".alert { color: white; background-color: red; padding: 20px; font-size: 24px; margin-top: 20px; }";
  html += ".btn { padding: 10px 20px; font-size: 20px; background-color: #333; color: white; border: none; border-radius: 5px; cursor: pointer; margin-top: 20px; }";
  html += "</style></head><body>";
  html += "<h1>ESP32 Emergency Alert System</h1>";

  if (alertTriggered) {
    html += "<div class='alert'>🚨 EMERGENCY ALERT 🚨</div>";
  } else {
    html += "<p>Status: Normal</p>";
  }

  html += "<form action='/trigger' method='GET'><button class='btn'>Trigger Alert</button></form>";
  html += "</body></html>";
  return html;
}

// Routes
void handleRoot() {
  server.send(200, "text/html", getHtml());
}

void handleTrigger() {
  alertTriggered = true;
  Serial.println("Alert triggered via web button!");
  server.sendHeader("Location", "/");  // Redirect back to root
  server.send(303);
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(115200);

  // Setup Access Point
  WiFi.softAP("ESP32_Alert", "12345678");
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Web routes
  server.on("/", handleRoot);
  server.on("/trigger", handleTrigger);

  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();

  // Hardware button press check
  if (digitalRead(BUTTON_PIN) == LOW) {
    alertTriggered = true;
    Serial.println("Hardware button pressed!");
    delay(500); // debounce
  }
}
