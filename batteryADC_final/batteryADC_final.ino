#include <Arduino.h>

// ---------------- CONFIG ----------------
const int ADC_PIN = 34;           // ADC pin connected to divider output
float DIV_FACTOR = 2.67466;       // resistor divider ratio (R1+R2)/R2
float CORR = 0.886019;            // calibration correction factor (7.82 / 8.826)
const int NUM_SAMPLES = 40;       // averaging samples
// Lookup table for 2S Li-ion
const float voltTable[] = {6.40, 6.80, 7.00, 7.20, 7.40, 7.60, 7.70, 7.80, 8.00, 8.20, 8.40};
const int   percTable[] = {   0,   10,   20,   30,   40,   60,   75,   85,   90,   95, 100};
const int tableSize = sizeof(percTable) / sizeof(percTable[0]);
// -----------------------------------------

// Read averaged ADC voltage
float readVadcAvg() {
  long sum = 0;
  for (int i = 0; i < NUM_SAMPLES; ++i) {
    sum += analogRead(ADC_PIN);
    delay(2);
  }
  float rawAvg = (float)sum / NUM_SAMPLES;
  float Vadc = (rawAvg / 4095.0) * 3.3;  // ADC count → voltage (nominal)
  return Vadc;
}

// Convert ADC → Vbat
float readVBat() {
  float Vadc = readVadcAvg();
  float Vbat = Vadc * DIV_FACTOR * CORR;
  return Vbat;
}

// Linear interpolation for % from voltage
int voltageToPercent(float vbat) {
  if (vbat <= voltTable[0]) return 0;
  if (vbat >= voltTable[tableSize - 1]) return 100;
  for (int i = 0; i < tableSize - 1; i++) {
    if (vbat >= voltTable[i] && vbat <= voltTable[i + 1]) {
      float slope = (percTable[i + 1] - percTable[i]) / (voltTable[i + 1] - voltTable[i]);
      return percTable[i] + (int)(slope * (vbat - voltTable[i]) + 0.5);
    }
  }
  return 0;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);                 // 12-bit ADC
  analogSetPinAttenuation(ADC_PIN, ADC_11db); // allows ~3.6V input on ADC
  delay(200);
  Serial.println("Battery monitor ready...");
}

void loop() {
  float vbat = readVBat();
  if (vbat > 8.45) vbat = 8.45;  // clamp safety max
  int pct = voltageToPercent(vbat);

  Serial.printf("Vbat = %.3f V  ->  %d%%   (DIV=%.6f CORR=%.6f)\n",
                vbat, pct, DIV_FACTOR, CORR);

  delay(1500);
}
