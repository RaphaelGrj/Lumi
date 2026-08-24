#pragma once
#include <Arduino.h>

// Portail captif WiFiManager (LUMI_CONFIGURATION) : collecte le SSID/mot de
// passe Wi-Fi ainsi que la clé API Gemini et le token Wit.ai (persistés via
// Preferences/NVS). Ne code jamais ces identifiants en dur.
void wifi_begin();

String wifi_getGeminiApiKey();
String wifi_getWitAiToken();
