#pragma once
#include <Arduino.h>

void audio_begin();
void audio_loop(); // à appeler à chaque tour de loop() pour faire avancer la lecture TTS

bool audio_isSpeaking();
void audio_speak(const String &text, const String &lang = "fr");

// Retourne true si un bruit fort vient d'être détecté (avec anti-rebond).
bool audio_detectTrigger();

// Enregistre 3s de micro et l'envoie à Wit.ai pour transcription (bloquant).
// Utilise le token renvoyé par wifi_getWitAiToken().
String audio_listenAndTranscribe();

// Compare (insensible à la casse) au mot de réveil défini par le build_flag
// WAKE_WORD dans platformio.ini.
bool audio_isWakeWord(const String &text);
