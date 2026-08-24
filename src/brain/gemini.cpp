#include "gemini.h"
#include "wifi.h"
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

String gemini_ask(const String &question, float contextTemp) {
  Serial.println("Demande à Gemini : " + question);

  WiFiClientSecure client;
  client.setInsecure(); // TODO (Prod) : client.setCACert(rootCACertificate);
  client.setTimeout(10000);
  HTTPClient http;

  String apiKey = wifi_getGeminiApiKey();
  apiKey.trim();
  // Remplacement du modèle preview par une version stable (1.5-flash)
  String url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + apiKey;
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");

  String contexte = "Tu es Lumi, un robot de bureau avec un style visuel 'Glitch'. "
                     "Il fait actuellement " +
                     String((int)contextTemp) + " degres dans la piece. "
                     "Tu dois repondre de maniere ultra-courte (1 a 2 phrases maximum). "
                     "L'humain te dit : ";

  // Utilisation sécurisée d'ArduinoJson pour la construction du Payload
  JsonDocument docPayload;
  docPayload["contents"][0]["parts"][0]["text"] = contexte + question;
  String payload;
  serializeJson(docPayload, payload);

  int httpResponseCode = http.POST(payload);
  String answer = "";

  if (httpResponseCode == 200) {
    String response = http.getString();
    JsonDocument doc;
    deserializeJson(doc, response);
    answer = doc["candidates"][0]["content"]["parts"][0]["text"].as<String>();
    answer.replace("*", "");
    Serial.println("Lumi répond : " + answer);
  } else {
    Serial.println("Erreur Gemini : " + String(httpResponseCode));
    Serial.println("Raison du refus : " + http.getString());
  }
  http.end();

  return answer;
}
