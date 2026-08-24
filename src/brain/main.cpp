#include <Arduino.h>
#include "behavior.h"
#include "display.h"
#include "sensors.h"
#include "audio.h"
#include "gemini.h"
#include "wifi.h"
#include "camera_link.h"

void setup() {
  Serial.begin(115200);
  Serial.println("[BOOT] ESP32-S3 started");

  sensors_begin();
  wifi_begin(); // portail Wi-Fi + clés Gemini/Wit.ai (voir wifi.cpp)
  display_begin();
  audio_begin();
  camera_link_begin();

  Serial.println("[GEMINI] Ready");
}

void loop() {
  audio_loop();

  if (behavior_isTalking() && !audio_isSpeaking()) {
    behavior_setIdle();
  }

  camera_link_update();

  if (!audio_isSpeaking()) {
    sensors_update();

    if (audio_detectTrigger()) {
      Serial.println("Bruit fort détecté !");
      behavior_setListening();

      String texteEntendu = audio_listenAndTranscribe();
      Serial.println("Tu as dit : " + texteEntendu);

      // Règle de conception : mot de réveil strict exigé (pas de préfixes)
      if (audio_isWakeWord(texteEntendu)) {
        String answer = gemini_ask(texteEntendu, sensors_getTemperature());
        if (answer != "") {
          audio_speak(answer);
          behavior_setTalking();
        } else {
          behavior_setIdle();
        }
      } else {
        behavior_setIdle();
      }
    }
  }

  delay(1);
}
