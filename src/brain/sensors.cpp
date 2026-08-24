#include "sensors.h"
#include "pins.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

static Adafruit_BME280 bme;
static Adafruit_MPU6050 mpu;
static bool bmeActif = false, mpuActif = false;
static unsigned long lastBMEReadTime = 0;
static const int bmeCheckInterval = 5000;
static float currentTemp = 20.0;

void sensors_begin() {
  Wire.begin(I2C_SDA, I2C_SCL);
  if (bme.begin(0x76)) {
    bmeActif = true;
    Serial.println("[BME280] OK");
  } else {
    Serial.println("[BME280] NOT FOUND");
  }
  if (mpu.begin()) {
    mpuActif = true;
    Serial.println("[MPU6050] OK");
  } else {
    Serial.println("[MPU6050] NOT FOUND");
  }
}

void sensors_update() {
  unsigned long currentMillis = millis();
  if (bmeActif && (currentMillis - lastBMEReadTime > (unsigned long)bmeCheckInterval)) {
    currentTemp = bme.readTemperature();
    lastBMEReadTime = currentMillis;
  }
  if (mpuActif) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    // La logique d'impact/secousse pourra être ajoutée ici
  }
}

float sensors_getTemperature() { return currentTemp; }
bool sensors_isBmeActive() { return bmeActif; }
bool sensors_isMpuActive() { return mpuActif; }
