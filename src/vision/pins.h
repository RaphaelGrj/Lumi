#pragma once

// --- Broches AI-THINKER (module caméra) ---
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// --- UART vers ESP32-S3 (brain) ---
// L'AI-Thinker n'a qu'un seul UART matériel exposé (UART0, GPIO1 TX / GPIO3
// RX), déjà utilisé par `Serial` pour le flash et les logs de boot. Il n'y a
// pas de second UART libre : le lien vers le brain PARTAGE donc `Serial`
// avec les logs de debug, exactement comme le faisait le code d'origine
// (Serial.printf("FACE:...")). C'est une contrainte du module, pas un choix :
// pas de pins dédiées à définir ici. Débrancher le lien S3 pour flasher.
