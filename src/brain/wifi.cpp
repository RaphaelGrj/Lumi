#include "wifi.h"
#include "pins.h"
#include "behavior.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h>

static Preferences preferences;
static String apiKey = "";
static String witAiToken = "";

static void configModeCallback(WiFiManager *myWiFiManager) {
  Serial.println("Portail activé !");
  behavior_setWifiPortal();
}

void wifi_begin() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  preferences.begin("lumi_config", false);
  apiKey = preferences.getString("gemini_key", "");
  witAiToken = preferences.getString("wit_key", "");
  pinMode(PIN_BOOT_BUTTON, INPUT_PULLUP);

  WiFiManagerParameter custom_gemini_key("gemini", "Cle API Gemini", apiKey.c_str(), 100);
  WiFiManagerParameter custom_wit_key("wit", "Token Wit.ai", witAiToken.c_str(), 60);
  WiFiManager wm;
  wm.setAPCallback(configModeCallback);
  wm.addParameter(&custom_gemini_key);
  wm.addParameter(&custom_wit_key);

  bool connected;
  if (digitalRead(PIN_BOOT_BUTTON) == LOW) {
    Serial.println("Bouton BOOT maintenu -> Ouverture forcée du portail");
    connected = wm.startConfigPortal("LUMI_CONFIGURATION");
  } else if (apiKey == "" || witAiToken == "") {
    connected = wm.startConfigPortal("LUMI_CONFIGURATION");
  } else {
    connected = wm.autoConnect("LUMI_CONFIGURATION");
  }

  if (connected) {
    Serial.println("[WIFI] Connected");
    behavior_setIdle();
  } else {
    Serial.println("[WIFI] ERROR");
  }

  String newGeminiKey = custom_gemini_key.getValue();
  String newWitKey = custom_wit_key.getValue();
  if (newGeminiKey != "") {
    apiKey = newGeminiKey;
    preferences.putString("gemini_key", apiKey);
  }
  if (newWitKey != "") {
    witAiToken = newWitKey;
    preferences.putString("wit_key", witAiToken);
  }
}

String wifi_getGeminiApiKey() { return apiKey; }
String wifi_getWitAiToken() { return witAiToken; }
