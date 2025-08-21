#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

#define RXD2 16
#define TXD2 17

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("GPS module serial started at 9600 baud");
}

void loop() {
  // Feed all available GPS data to TinyGPSPlus
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isValid()) {
    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);

    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);

    if (gps.altitude.isValid()) {
      Serial.print("Altitude: ");
      Serial.print(gps.altitude.meters());
      Serial.println(" m");
    }
    Serial.println("-----------------------------");
  } else {
    Serial.println("Waiting for valid GPS signal...\n-----------------------------");
  }

  delay(1000);
}
