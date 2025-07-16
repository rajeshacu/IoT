#include <SPI.h>
#include <SD.h>

const int chipSelect = 5;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("Initializing SD card...");
    
    // Initialize with explicit SPI pins
    SPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, CS
    
    if (!SD.begin(chipSelect)) {
        Serial.println("initialization failed!");
        Serial.println("Check:");
        Serial.println("- SD card is inserted");
        Serial.println("- Wiring is correct");
        Serial.println("- Card is FAT32 formatted");
        return;
    }
    
    Serial.println("initialization done.");
    
    // Simple write test
    File myFile = SD.open("/test.txt", FILE_WRITE);
    if (myFile) {
        Serial.println("Writing to test.txt...");
        myFile.println("Hello ESP32 with SD Card Module!");
        myFile.close();
        Serial.println("done.");
    } else {
        Serial.println("error opening test.txt");
    }
    
    // Simple read test
    myFile = SD.open("/test.txt");
    if (myFile) {
        Serial.println("test.txt contents:");
        while (myFile.available()) {
            Serial.write(myFile.read());
        }
        myFile.close();
    } else {
        Serial.println("error opening test.txt");
    }
}

void loop() {
    // Nothing happens after setup
}
