#include <SPI.h>
#include <SD.h>

const int CS = 5;

void setup() {
  Serial.begin(115200);
  delay(2000);
  SPI.begin(18, 19, 23, CS);
  Serial.println("Initializing SD card...");
  if (SD.begin(CS)) {
    Serial.println("SD card initialized successfully!");
  } else {
    Serial.println("Initialization failed!");
  }
}

void loop() {
  // Empty
}
