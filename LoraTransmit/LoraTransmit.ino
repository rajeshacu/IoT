/*
 * Simple LoRa ESP32 Sender - Diagnostic Version
 * Use this to verify if LoRa module is working and sending messages
 * 
 * Hardware: ESP32 + LoRa module (SX1276/SX1278)
 * Library: LoRa by Sandeep Mistry
 * 
 * Wiring:
 * LoRa Module -> ESP32
 * VCC -> 3.3V
 * GND -> GND
 * SCK -> GPIO 5
 * MISO -> GPIO 19
 * MOSI -> GPIO 27
 * NSS (CS) -> GPIO 18
 * RST -> GPIO 14
 * DIO0 -> GPIO 26
 */

#include <SPI.h>
#include <LoRa.h>

// LoRa pin definitions
#define SS 18
#define RST 14
#define DIO0 26

// Counter variable
int counter = 0;

void setup() {
  Serial.begin(9600);
  delay(2000);
  
  Serial.println();
  Serial.println("=== LoRa Diagnostic Test ===");
  Serial.println("Step 1: Checking pins...");
  
  // Test pin setup
  pinMode(SS, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(DIO0, INPUT);
  
  Serial.println("Step 2: Resetting LoRa module...");
  digitalWrite(RST, LOW);
  delay(10);
  digitalWrite(RST, HIGH);
  delay(10);
  
  Serial.println("Step 3: Setting up SPI...");
  SPI.begin();
  
  Serial.println("Step 4: Configuring LoRa pins...");
  LoRa.setPins(SS, RST, DIO0);
  
  Serial.println("Step 5: Attempting LoRa initialization...");
  
  // Try multiple times with different approaches
  bool success = false;
  for (int attempt = 1; attempt <= 3; attempt++) {
    Serial.print("Attempt ");
    Serial.print(attempt);
    Serial.print(": ");
    
    if (LoRa.begin(433E6)) {
      Serial.println("SUCCESS!");
      success = true;
      break;
    } else {
      Serial.println("Failed");
      delay(1000);
    }
  }
  
  if (!success) {
    Serial.println("ERROR: All attempts failed!");
    Serial.println("Possible issues:");
    Serial.println("1. Check wiring (especially NSS/CS, RST, DIO0)");
    Serial.println("2. Check power supply (3.3V stable)");
    Serial.println("3. Check LoRa module is SX1276/SX1278");
    Serial.println("4. Try different frequency: 915E6 or 868E6");
    Serial.println("Entering diagnostic loop...");
    
    // Diagnostic loop instead of infinite hang
    while (1) {
      Serial.println("Still stuck - check hardware!");
      delay(5000);
    }
  }
  
  Serial.println("LoRa module working! Configuring...");
  
  // Simple configuration
  LoRa.setTxPower(17);
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(8);
  LoRa.setSyncWord(0x12);
  
  Serial.println("Configuration complete!");
  Serial.println("Starting transmission test...");
  Serial.println("============================");
}

void loop() {
  Serial.print("Attempting to send: Hello ");
  Serial.print(counter);
  Serial.print(" ... ");
  
  // Send packet
  LoRa.beginPacket();
  LoRa.print("Hello ");
  LoRa.print(counter);
  bool success = LoRa.endPacket();
  
  if (success) {
    Serial.println("SENT successfully!");
  } else {
    Serial.println("FAILED to send!");
  }
  
  counter++;
  
  // Wait 3 seconds before next transmission
  delay(3000);
}