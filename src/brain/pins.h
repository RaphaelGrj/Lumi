#pragma once

// --- TFT (ST7789, yeux) ---
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS   10
#define TFT_DC   13
#define TFT_RST  14

// --- I2S microphone (INMP441) ---
#define I2S_SCK_MIC 4
#define I2S_WS_MIC  5
#define I2S_SD_MIC  6

// --- I2S ampli haut-parleur (MAX98357A) ---
#define I2S_BCLK_AMP 15
#define I2S_LRC_AMP  16
#define I2S_DIN_AMP  17

// --- I2C (BME280 + MPU6050) ---
#define I2C_SDA 8
#define I2C_SCL 9

// --- Bouton BOOT (force le portail de config) ---
#define PIN_BOOT_BUTTON 0

// --- UART vers ESP32-CAM ---
// PROPOSÉ, à confirmer avant soudure : rien n'est câblé physiquement pour ce
// lien aujourd'hui. GPIO43/44 = UART0 par défaut du DevKitC-1, libres ici car
// ARDUINO_USB_CDC_ON_BOOT=1 fait passer Serial (debug) par l'USB natif.
#define CAM_UART_TX 43
#define CAM_UART_RX 44
