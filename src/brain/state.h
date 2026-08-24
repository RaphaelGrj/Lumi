#pragma once

enum class EmotionState {
  BOOT_GLITCH,  // texte "LUMI" glitché à l'écran, au démarrage
  IDLE,         // yeux au repos, mouvement/blink aléatoires
  LISTENING,    // point cyan d'écoute active (micro déclenché)
  TALKING,      // animation de bouche (TTS en cours)
  WIFI_PORTAL   // portail de configuration Wi-Fi ouvert
};

// Partagé entre la tâche d'affichage et loop() : garder volatile.
// Seul behavior.cpp doit écrire cette variable.
extern volatile EmotionState currentState;
