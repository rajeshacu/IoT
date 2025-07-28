#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define SD_CS 5  // Chip Select pin (connect CS of SD card to GPIO 5)

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("📁 Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("❌ Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("❌ Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("📂 DIR : ");
      Serial.println(file.name());
      if (levels > 0) {
        listDir(fs, file.name(), levels - 1);  // Recursive listing
      }
    } else {
      Serial.print("📄 FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("🔧 Starting SD card initialization...");

  if (!SD.begin(SD_CS)) {
    Serial.println("❌ SD card mount failed. Check wiring and CS pin.");
    return;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("❌ No SD card attached.");
    return;
  }

  Serial.print("✅ SD Card Type: ");
  if (cardType == CARD_MMC) Serial.println("MMC");
  else if (cardType == CARD_SD) Serial.println("SDSC");
  else if (cardType == CARD_SDHC) Serial.println("SDHC");
  else Serial.println("Unknown");

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("📦 Card Size: %llu MB\n", cardSize);

  // Now list files and directories
  listDir(SD, "/", 2);  // "/" = root, "2" = recurse up to 2 levels
}

void loop() {
  // Nothing here
}
