#include <SPI.h>
#include <LoRa.h>

#define SS    18
#define RST   14
#define DIO0  26
#define BAND  433E6  // 433 MHz

void setup() {
  Serial.begin(115200);
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    Serial.println("LoRa init failed. Check wiring.");
    while (true);
  }

  Serial.println("LoRa Receiver Started");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String message = "";
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }

    Serial.print("Received: ");
    Serial.println(message);
  }
}
