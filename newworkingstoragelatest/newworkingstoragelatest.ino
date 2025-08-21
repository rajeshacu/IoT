#include <SPI.h>
#include <SD.h>
#include <LoRa.h>

// --- LoRa (VSPI) Pins ---
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23
#define LORA_CS    5
#define LORA_RST   14
#define LORA_DIO0  2

// --- SD Card (HSPI) Pins ---
#define SD_CS    13
#define SD_SCK   25
#define SD_MISO  26
#define SD_MOSI  27

// --- HSPI instance for SD card ---
SPIClass hspi(HSPI);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // --- LoRa Init ---
  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, LOW);
  delay(10);
  digitalWrite(LORA_RST, HIGH);
  delay(100);

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI); // VSPI for LoRa
  LoRa.setSPIFrequency(1E6);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  Serial.println("Initializing LoRa...");
  if (!LoRa.begin(433E6)) {
    Serial.println("✗ LoRa init failed!");
    while (1);
  }
  Serial.println("✓ LoRa initialized.");

  // --- SD Card Init ---
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

    // --- Append to data.txt ---
    File dataFile = SD.open("/data.txt", FILE_APPEND);
    if (dataFile) {
      dataFile.println(data);
      dataFile.close();
      Serial.println("✓ Appended to data.txt");
    } else {
      Serial.println("✗ Failed to write to data.txt");
    }

    // --- Overwrite latest.json ---
    File latestFile = SD.open("/latest.json", FILE_WRITE);
    if (latestFile) {
      latestFile.print(data); // NOT println - to avoid newline in JSON
      latestFile.close();
      Serial.println("✓ Overwritten latest.json");
    } else {
      Serial.println("✗ Failed to write to latest.json");
    }
  }

  delay(10); // For stability
}
