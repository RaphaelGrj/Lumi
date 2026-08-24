#pragma once
#include <Arduino.h>

// Interroge Gemini avec la question posée par l'utilisateur, en donnant la
// température ambiante comme contexte. Retourne la réponse texte, ou ""
// en cas d'échec (erreur déjà loggée sur Serial).
String gemini_ask(const String &question, float contextTemp);
