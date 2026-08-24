#include "protocol.h"
#include <Arduino.h>

static const unsigned long presenceTimeoutMs = 2500;
static bool presenceReported = false;
static unsigned long lastMotionTime = 0;

void protocol_begin() {
  Serial.println("[CAM] UART ready");
}

void protocol_reportPresence(bool motionDetected) {
  unsigned long now = millis();
  if (motionDetected) {
    lastMotionTime = now;
    if (!presenceReported) {
      presenceReported = true;
      Serial.println("P,1");
    }
  } else if (presenceReported && (now - lastMotionTime > presenceTimeoutMs)) {
    presenceReported = false;
    Serial.println("P,0");
  }
}

void protocol_reportGaze(int x, int y) {
  Serial.print("T,");
  Serial.print(x);
  Serial.print(",");
  Serial.println(y);
}
