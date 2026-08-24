#include "camera_link.h"
#include "pins.h"
#include <Arduino.h>

static bool hasPresence = false;
static int gazeX = 0;
static int gazeY = 0;

void camera_link_begin() {
  Serial1.begin(115200, SERIAL_8N1, CAM_UART_RX, CAM_UART_TX);
  Serial.println("[CAM] UART ready");
}

void camera_link_update() {
  while (Serial1.available()) {
    String line = Serial1.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    if (line.startsWith("T,")) {
      int firstComma = line.indexOf(',');
      int secondComma = line.indexOf(',', firstComma + 1);
      if (secondComma == -1) {
        Serial.println("[CAM] invalid: " + line);
        continue;
      }
      gazeX = line.substring(firstComma + 1, secondComma).toInt();
      gazeY = line.substring(secondComma + 1).toInt();
    } else if (line.startsWith("P,")) {
      String value = line.substring(2);
      if (value == "1") {
        hasPresence = true;
      } else if (value == "0") {
        hasPresence = false;
      } else {
        Serial.println("[CAM] invalid: " + line);
      }
    } else {
      Serial.println("[CAM] invalid: " + line);
    }
  }
}

bool camera_hasPresence() { return hasPresence; }
int camera_gazeX() { return gazeX; }
int camera_gazeY() { return gazeY; }
