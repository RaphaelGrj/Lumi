#include "audio.h"
#include "pins.h"
#include "wifi.h"
#include <Arduino.h>
#include <driver/i2s.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Audio.h" // Bibliothèque Schreibfaul1 (v2.0.6 requise selon configuration PIO)

static Audio audio;

static const int SEUIL_BRUIT_REVEIL = 2500;
static unsigned long lastTriggerTime = 0;
static const int DEBOUNCE_AUDIO_MS = 4000; // Délai anti-rebond pour le micro

// Buffer Audio (Allocation PSRAM pour préserver la RAM interne)
static const int recordTimeMs = 3000;
static const int sampleRate = 16000;
static const int numSamples = (recordTimeMs * sampleRate) / 1000;
static int16_t *audioBuffer = nullptr;

// Certificats Root CA (À remplir pour la production, remplace setInsecure)
static const char *rootCACertificate =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4G\n"
    "... Remplacer par le vrai certificat racine GlobalSign ...\n"
    "-----END CERTIFICATE-----\n";

static void setupI2S() {
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = 16000,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = 1024,
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0};
  i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_SCK_MIC,
      .ws_io_num = I2S_WS_MIC,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = I2S_SD_MIC};
  i2s_driver_install(I2S_NUM_1, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_1, &pin_config);
}

void audio_begin() {
  audioBuffer = (int16_t *)ps_malloc(numSamples * sizeof(int16_t));
  if (!audioBuffer) {
    Serial.println("[MIC] ERROR - PSRAM alloc failed");
  }

  audio.setPinout(I2S_BCLK_AMP, I2S_LRC_AMP, I2S_DIN_AMP);
  audio.setVolume(15);
  Serial.println("[SPEAKER] OK");

  setupI2S();
  Serial.println("[MIC] OK");
}

void audio_loop() { audio.loop(); }

bool audio_isSpeaking() { return audio.isRunning(); }

void audio_speak(const String &text, const String &lang) {
  audio.connecttospeech(text.c_str(), lang.c_str());
}

bool audio_detectTrigger() {
  size_t bytesRead = 0;
  int16_t sampleBuffer[256];
  i2s_read(I2S_NUM_1, &sampleBuffer, sizeof(sampleBuffer), &bytesRead, portMAX_DELAY);

  const size_t sampleCount = bytesRead / sizeof(sampleBuffer[0]);
  int average = 0;
  if (sampleCount > 0) {
    int32_t sum = 0;
    for (size_t i = 0; i < sampleCount; i++) {
      sum += abs(sampleBuffer[i]);
    }
    average = sum / sampleCount;
  }

  unsigned long currentMillis = millis();
  if (average > SEUIL_BRUIT_REVEIL && (currentMillis - lastTriggerTime > (unsigned long)DEBOUNCE_AUDIO_MS)) {
    lastTriggerTime = currentMillis;
    return true;
  }
  return false;
}

String audio_listenAndTranscribe() {
  Serial.println("Enregistrement en cours (3 sec)...");

  if (!audioBuffer) {
    Serial.println("Erreur: Buffer PSRAM non alloué !");
    return "";
  }

  size_t bytesRead = 0;
  i2s_read(I2S_NUM_1, audioBuffer, numSamples * sizeof(int16_t), &bytesRead, portMAX_DELAY);
  Serial.println("Envoi à Wit.ai...");

  WiFiClientSecure client;
  client.setInsecure(); // TODO (Prod) : Remplacer par client.setCACert(rootCACertificate);
  client.setTimeout(10000);
  HTTPClient http;

  if (!http.begin(client, "https://api.wit.ai/speech?v=20230215")) {
    Serial.println("Erreur: Impossible de démarrer la connexion HTTPS");
    return "";
  }

  http.addHeader("Authorization", "Bearer " + wifi_getWitAiToken());
  http.addHeader("Content-Type", "audio/raw;encoding=signed-integer;bits=16;rate=16000;endian=little");

  int httpResponseCode = http.POST((uint8_t *)audioBuffer, bytesRead);
  String response = "";

  if (httpResponseCode == 200) {
    response = http.getString();
    Serial.println("Réponse Wit.ai reçue !");
  } else {
    Serial.println("Erreur Wit.ai : Code HTTP " + String(httpResponseCode));
    if (httpResponseCode < 0) {
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }
  }

  http.end();

  String texteExtrait = "";
  if (response.length() > 0) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);
    if (!error) {
      texteExtrait = doc["text"].as<String>();
      texteExtrait.toLowerCase();
    } else {
      Serial.println("Erreur de lecture du JSON Wit.ai");
    }
  }
  return texteExtrait;
}

bool audio_isWakeWord(const String &text) {
  String wakeWord = String(WAKE_WORD);
  wakeWord.toLowerCase();
  return text == wakeWord;
}
