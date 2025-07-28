#include <SPI.h>
#include <SD.h>

#define SD_CS_PIN 13
#define LORA_CS_PIN 5

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Ensure LoRa is deselected
  pinMode(LORA_CS_PIN, OUTPUT);
  digitalWrite(LORA_CS_PIN, HIGH);
  
  // Initialize SD card CS pin
  pinMode(SD_CS_PIN, OUTPUT);
  digitalWrite(SD_CS_PIN, HIGH);
  
  Serial.println("Initializing SD card...");
  
  // Initialize with explicit SPI pins
  SPI.begin(18, 19, 23, 13);
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  
  Serial.println("SD card initialized successfully!");
  
  // Test card info
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %llu MB\n", cardSize);
  
  // Test file operations
  testFileOperations();
}

void testFileOperations() {
  Serial.println("\n--- Testing File Operations ---");
  
  // Write test
  File file = SD.open("/test.txt", FILE_WRITE);
  if (file) {
    file.println("Hello from ESP32!");
    file.printf("Timestamp: %lu\n", millis());
    file.close();
    Serial.println("✓ File write successful");
  } else {
    Serial.println("✗ File write failed");
  }
  
  // Read test
  file = SD.open("/test.txt");
  if (file) {
    Serial.println("✓ File content:");
    while (file.available()) {
      Serial.write(file.read());
    }
    file.close();
  } else {
    Serial.println("✗ File read failed");
  }
  
  // Directory listing
  Serial.println("\n--- Root Directory ---");
  File root = SD.open("/");
  File entry = root.openNextFile();
  while (entry) {
    Serial.printf("File: %s, Size: %d bytes\n", entry.name(), entry.size());
    entry.close();
    entry = root.openNextFile();
  }
  root.close();
}

void loop() {
  delay(5000);
}