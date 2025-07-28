#include <SPI.h>
#include <SD.h>
#include <LoRa.h>

// --- LoRa (VSPI) ---
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23
#define LORA_CS    5
#define LORA_RST   14
#define LORA_DIO0  2

// --- SD Card (HSPI) ---
#define SD_CS    13
#define SD_SCK   25
#define SD_MISO  26
#define SD_MOSI  27

SPIClass hspi(HSPI); // Separate SPI instance for SD card (HSPI)

void setup() {
  Serial.begin(115200);
  delay(1000);

  // --- Reset LoRa ---
  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, LOW);
  delay(10);
  digitalWrite(LORA_RST, HIGH);
  delay(100);

  // --- Begin VSPI for LoRa ---
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI); 
  LoRa.setSPIFrequency(1E6);  // Optional: slow down SPI for reliability
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  Serial.println("Initializing LoRa...");
  if (!LoRa.begin(433E6)) {
    Serial.println("✗ LoRa init failed!");
    while (1);
  }
  Serial.println("✓ LoRa initialized.");

  // --- Begin HSPI for SD card ---
  Serial.println("Initializing SD card...");
  hspi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, hspi)) {
    Serial.println("✗ SD card init failed!");
    while (1);
  }
  Serial.println("✓ SD card initialized.");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize > 0) {
    Serial.println("\n--- LoRa Packet Received ---");

    String data = "";
    while (LoRa.available()) {
      data += (char)LoRa.read();
    }

    Serial.println("Received: " + data);

    // --- Save to SD card ---
    File file = SD.open("/data.txt", FILE_APPEND);
    if (file) {
      file.println(data);
      file.close();
      Serial.println("✓ Saved to SD card");
    } else {
      Serial.println("✗ Failed to write to SD card");
    }
  }

  delay(10); // Allow watchdog timer to reset
}
