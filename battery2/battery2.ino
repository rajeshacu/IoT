#include <Arduino.h>

const int BATTERY_PIN = 34;      // ADC1 pin
const float VREF = 3.3;
const float R1 = 150000.0;
const float R2 = 100000.0;
const float SCALE = (R1 + R2) / R2;     // 2.5

// Calibration (from your 7.82 V vs ~7.745 V)
const float CAL_SCALE  = 1.0097f;       // gain correction ~+0.97%
const float CAL_OFFSET = 0.00f;         // add later if you do 2-point cal

float pctFromVoltage(float v) {
  if (v <= 6.00) return 0;
  if (v >= 8.40) return 100;
  struct P { float v, p; };
  static const P T[] = {
    {8.40,100},{8.20,90},{8.00,80},{7.80,70},{7.60,60},{7.40,50},
    {7.20,40},{7.00,30},{6.80,20},{6.60,10},{6.40,5},{6.00,0}
  };
  for (int i = 0; i < (int)(sizeof(T)/sizeof(T[0]))-1; ++i) {
    if (v >= T[i+1].v) {
      float t = (v - T[i+1].v) / (T[i].v - T[i+1].v);
      return T[i+1].p + t * (T[i].p - T[i+1].p);
    }
  }
  return 0;
}

uint16_t readAdcRaw(int samples = 32) {
#if defined(ESP32)
  analogReadResolution(12);
  analogSetPinAttenuation(BATTERY_PIN, ADC_11db);  // ~0–3.5V
#endif
  uint32_t acc = 0;
  for (int i = 0; i < samples; ++i) { acc += analogRead(BATTERY_PIN); delayMicroseconds(200); }
  return (uint16_t)(acc / samples);
}

void setup() {
  Serial.begin(115200);
  delay(200);
}

void loop() {
  uint16_t adcRaw = readAdcRaw(64);                 // average for stability
  float vAdc  = (adcRaw / 4095.0f) * VREF;         // pin voltage
  float vBatU = vAdc * SCALE;                      // uncalibrated pack volts
  float vBat  = vBatU * CAL_SCALE + CAL_OFFSET;    // calibrated pack volts
  float pct   = pctFromVoltage(vBat);

  if (pct < 0) pct = 0; if (pct > 100) pct = 100;

  Serial.print("ADCraw: "); Serial.print(adcRaw);
  Serial.print(" | V(uncal): "); Serial.print(vBatU, 3);
  Serial.print(" | V: "); Serial.print(vBat, 3);
  Serial.print(" V  |  "); Serial.print(pct, 1); Serial.println(" %");
  delay(2000);
}
