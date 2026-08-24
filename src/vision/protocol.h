#pragma once

// Le lien vers le brain partage `Serial` (UART0) avec les logs de debug —
// voir pins.h. `Serial.begin()` est déjà fait dans main.cpp avant l'appel ;
// cette fonction se contente de logguer que le lien est prêt.
void protocol_begin();

// Émet P,1 / P,0 uniquement au changement d'état (latch avec timeout —
// repasse à P,0 si aucun mouvement n'est vu pendant presenceTimeoutMs).
void protocol_reportPresence(bool motionDetected);

// Émet T,x,y.
void protocol_reportGaze(int x, int y);
