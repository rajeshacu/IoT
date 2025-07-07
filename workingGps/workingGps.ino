// Define the RX and TX pins for Serial 2
#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600

// Create an instance of the HardwareSerial class for Serial 2
HardwareSerial gpsSerial(2);

void setup() {
  // Start Serial Monitor
  Serial.begin(115200);
  // Start Serial 2 with the defined RX and TX pins and a baud rate of 9600
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial 2 started at 9600 baud rate");
}

void loop() {
  while (gpsSerial.available() > 0) {
    // Read the byte data from the GPS
    char gpsData = gpsSerial.read();
    Serial.print(gpsData);
  }
}
