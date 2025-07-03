#include <SPI.h>
#include <LoRa.h>

// Pin definitions
#define SS   5
#define RST  14
#define DIO0 2

int counter = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Sender");

  LoRa.setPins(SS, RST, DIO0);

  // Set frequency: 433E6 for Asia, 868E6 for Europe, 915E6 for North America
  while (!LoRa.begin(433E6)) { // Change to your region's frequency
    Serial.println(".");
    delay(500);
  }

  LoRa.setSyncWord(0xF3); // Sync word must match on sender/receiver
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;
  delay(10000); // Send every 10 seconds
}
