#include <Arduino.h>
#include "motion.h"
#include "protocol.h"

void setup() {
  Serial.begin(115200);
  motion_begin();
  protocol_begin();
}

void loop() {
  MotionResult motion = motion_detect();

  protocol_reportPresence(motion.detected);
  if (motion.detected) {
    protocol_reportGaze(motion.gazeX, motion.gazeY);
  }

  delay(80);
}
