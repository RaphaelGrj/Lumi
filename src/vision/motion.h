#pragma once
#include <Arduino.h>

// Initialise la caméra AI-Thinker et le buffer de frame précédente.
void motion_begin();

struct MotionResult {
  bool detected;
  int gazeX; // mappé -35..35
  int gazeY; // mappé -20..20
};

// Capture une frame et la compare à la précédente (différence de pixels).
MotionResult motion_detect();
