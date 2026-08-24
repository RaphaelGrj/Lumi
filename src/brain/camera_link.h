#pragma once

// Ouvre le lien UART vers l'ESP32-CAM (broches dans pins.h — proposées, à
// confirmer avant soudure, rien n'est câblé aujourd'hui).
void camera_link_begin();

// À appeler à chaque tour de loop() : lit et parse les lignes disponibles
// (T,x,y / P,0 / P,1). Les lignes invalides sont loggées, jamais ignorées
// silencieusement.
void camera_link_update();

bool camera_hasPresence();
int camera_gazeX();
int camera_gazeY();
