#include <SPI.h>
#include <LoRa.h>

// === Pin definitions for RA-02 module on ESP32 ===
#define SS_PIN    5     // NSS/CS pin (GPIO 5)
#define RST_PIN   14    // Reset pin (GPIO 14)
#define DIO0_PIN  2     // DIO0 pin for interrupt (GPIO 2)

// === LoRa configuration ===
#define FREQUENCY 433E6  // 433 MHz
#define BANDWIDTH 125E3  // 125 kHz
#define SPREADING_FACTOR 7
#define CODING_RATE 5    // 4/5
#define SYNC_WORD 0x12
#define TX_POWER 17      // Max power for SX1278

// === Mode selection ===
#define TEST_MODE_SENDER 1   // Set to 1 for sender, 0 for receiver
#define TEST_INTERVAL 3000   // Sending interval in milliseconds

// === Variables ===
bool moduleFound = false;
int packetCounter = 0;
unsigned long lastSendTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("LoRa Hello Message Test");
  Serial.println("========================");

  // Initialize SPI
  SPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, SS

  // Initialize LoRa
  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);

  Serial.print("Initializing LoRa module... ");
  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("FAILED!");
    printConnectionGuide();
    while (1);
  }

  Serial.println("SUCCESS!");
  moduleFound = true;

  // LoRa settings
  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setSignalBandwidth(BANDWIDTH);
  LoRa.setCodingRate4(CODING_RATE);
  LoRa.setSyncWord(SYNC_WORD);
  LoRa.setTxPower(TX_POWER);

#if TEST_MODE_SENDER
  Serial.println("\n🚀 SENDER MODE ACTIVE");
#else
  Serial.println("\n📡 RECEIVER MODE ACTIVE");
#endif

  Serial.println("Module ready. Starting operations...\n");
}

void loop() {
  if (!moduleFound) return;

#if TEST_MODE_SENDER
  senderMode();
#else
  receiverMode();
#endif
}

// === Sender Mode ===
void senderMode() {
  if (millis() - lastSendTime > TEST_INTERVAL) {
    packetCounter++;

    String message = "Hello #" + String(packetCounter);

    Serial.println("=== SENDING PACKET ===");
    Serial.printf("Message: %s\n", message.c_str());

    unsigned long startTime = millis();

    LoRa.beginPacket();
    LoRa.print(message);
    int result = LoRa.endPacket();

    unsigned long duration = millis() - startTime;

    if (result == 1) {
      Serial.printf("✓ Sent successfully in %lu ms\n", duration);
    } else {
      Serial.println("✗ Transmission failed!");
    }

    Serial.println("======================\n");

    lastSendTime = millis();
  }
}

// === Receiver Mode ===
void receiverMode() {
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    Serial.println("=== PACKET RECEIVED ===");
    Serial.printf("Size: %d bytes\n", packetSize);

    String receivedMessage = "";
    while (LoRa.available()) {
      receivedMessage += (char)LoRa.read();
    }

    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();
    long freqError = LoRa.packetFrequencyError();

    Serial.printf("Content: %s\n", receivedMessage.c_str());
    Serial.printf("RSSI: %d dBm\n", rssi);
    Serial.printf("SNR: %.2f dB\n", snr);
    Serial.printf("Freq Error: %ld Hz\n", freqError);

    // Signal quality level
    if (rssi > -80) {
      Serial.println("📶 Signal: EXCELLENT");
    } else if (rssi > -100) {
      Serial.println("📶 Signal: GOOD");
    } else if (rssi > -120) {
      Serial.println("📶 Signal: FAIR");
    } else {
      Serial.println("📶 Signal: POOR");
    }

    Serial.println("=======================\n");
  }
}

// === Helper function to show connection tips ===
void printConnectionGuide() {
  Serial.println("\n❌ MODULE INITIALIZATION FAILED!");
  Serial.println("\n🔌 Check your connections:");
  Serial.println("LoRa RA-02    ESP32");
  Serial.println("----------    -----");
  Serial.println("VCC       ->  3.3V");
  Serial.println("GND       ->  GND");
  Serial.println("MOSI      ->  GPIO 23");
  Serial.println("MISO      ->  GPIO 19");
  Serial.println("SCK       ->  GPIO 18");
  Serial.println("NSS (CS)  ->  GPIO 5");
  Serial.println("RST       ->  GPIO 14");
  Serial.println("DIO0      ->  GPIO 2");
  Serial.println("\n⚠️  Use a 433 MHz antenna and 3.3V power only.");
}
