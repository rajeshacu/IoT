#include <SPI.h>
#include <LoRa.h>

#define SS 5      // LoRa radio chip select
#define RST 14    // LoRa radio reset
#define DIO0 2    // LoRa radio DIO0

int counter = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Sender");

  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) { // Use 433E6 for 433 MHz
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // Optional: Set sync word to match receiver (default is 0x34)
  // LoRa.setSyncWord(0xF3);
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;
  delay(2000); // Send every 2 seconds
}
