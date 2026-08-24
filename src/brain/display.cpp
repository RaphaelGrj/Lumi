#include "display.h"
#include "pins.h"
#include "state.h"
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <WiFi.h>

static Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
static TaskHandle_t displayTaskHandle;

// Géométrie des yeux
static const int baseEyeWidth = 70, baseEyeHeight = 100, irisOffset = 22;
static const int leftEyeBaseX = 50, rightEyeBaseX = 160, baseY = 70;
static float currentXOffset = 0, currentYOffset = 0, targetXOffset = 0, targetYOffset = 0;
static unsigned long lastMoveTime = 0, lastBlinkTime = 0;
static int nextMoveDelay = 2000, nextBlinkDelay = 3000;
static bool isBlinking = false;
static float eyeHeightScale = 1.0;

static void dessinerIconeWifi(int x, int y, uint16_t color) {
  tft.fillCircle(x + 6, y + 10, 1, color);
  tft.drawCircleHelper(x + 6, y + 9, 4, 1 | 2, color);
  tft.drawCircleHelper(x + 6, y + 9, 8, 1 | 2, color);
}

// CŒUR 0 : AFFICHAGE & YEUX GLITCH
static void displayTask(void *pvParameters) {
  for (;;) {
    unsigned long currentTime = millis();
    EmotionState state = currentState;

    if (state == EmotionState::BOOT_GLITCH) {
      tft.fillScreen(ST77XX_BLACK);
      tft.setTextSize(4);
      int x = 70, y = 120;
      // Couleurs Cyan (0x07FF) et Magenta (0xF81F) de l'identité visuelle
      tft.setTextColor(0x07FF); tft.setCursor(x + random(-4, 5), y + random(-1, 2)); tft.print("LUMI");
      tft.setTextColor(0xF81F); tft.setCursor(x + random(-4, 5), y + random(-1, 2)); tft.print("LUMI");
      tft.setTextColor(ST77XX_WHITE); tft.setCursor(x, y); tft.print("LUMI");
      for (int i = 0; i < 8; i++) {
        tft.fillRect(random(0, 240), random(0, 280), random(20, 80), random(2, 6), ST77XX_BLACK);
        if (random(100) > 85) tft.fillRect(random(0, 240), random(0, 280), random(5, 30), random(1, 3), ST77XX_WHITE);
      }
      delay(120);
      continue;
    }

    if (currentTime - lastMoveTime > (unsigned long)nextMoveDelay) {
      targetXOffset = random(-30, 31); targetYOffset = random(-15, 16);
      lastMoveTime = currentTime; nextMoveDelay = random(1500, 4000);
    }
    if (!isBlinking && (currentTime - lastBlinkTime > (unsigned long)nextBlinkDelay)) { isBlinking = true; }
    if (isBlinking) {
      eyeHeightScale -= 0.25;
      if (eyeHeightScale <= 0.1) { eyeHeightScale = 0.1; isBlinking = false; lastBlinkTime = currentTime; nextBlinkDelay = random(2000, 6000); }
    } else {
      if (eyeHeightScale < 1.0) eyeHeightScale += 0.15;
      if (eyeHeightScale > 1.0) eyeHeightScale = 1.0;
    }
    currentXOffset += (targetXOffset - currentXOffset) * 0.2;
    currentYOffset += (targetYOffset - currentYOffset) * 0.2;

    tft.fillScreen(ST77XX_BLACK);
    if (WiFi.status() == WL_CONNECTED) {
      dessinerIconeWifi(218, 8, 0x07FF);
    }

    int currentHeight = baseEyeHeight * eyeHeightScale;
    int currentY = baseY + currentYOffset + ((baseEyeHeight - currentHeight) / 2);
    int lX = leftEyeBaseX + currentXOffset, rX = rightEyeBaseX + currentXOffset;
    int glitchOffset = 4;
    tft.fillRoundRect(lX - glitchOffset, currentY, baseEyeWidth, currentHeight, 20, 0x07FF);
    tft.fillRoundRect(lX + glitchOffset, currentY, baseEyeWidth, currentHeight, 20, 0xF81F);
    tft.fillRoundRect(lX, currentY, baseEyeWidth, currentHeight, 20, ST77XX_WHITE);
    tft.fillRoundRect(rX - glitchOffset, currentY, baseEyeWidth, currentHeight, 20, 0x07FF);
    tft.fillRoundRect(rX + glitchOffset, currentY, baseEyeWidth, currentHeight, 20, 0xF81F);
    tft.fillRoundRect(rX, currentY, baseEyeWidth, currentHeight, 20, ST77XX_WHITE);
    int lIrisH = currentHeight - (irisOffset * 2), rIrisH = currentHeight - (irisOffset * 2), irisW = baseEyeWidth - (irisOffset * 2);
    if (lIrisH > 0) tft.fillRoundRect(lX + irisOffset, currentY + irisOffset, irisW, lIrisH, 8, ST77XX_BLACK);
    if (rIrisH > 0) tft.fillRoundRect(rX + irisOffset, currentY + irisOffset, irisW, rIrisH, 8, ST77XX_BLACK);

    if (state == EmotionState::LISTENING) {
      tft.fillCircle(120, 240, 8, 0x07FF); // Point cyan d'écoute active
    }

    if (state == EmotionState::TALKING) {
      int mouthY = 210, blockSize = 12, spacing = 3, rows = random(2, 4);
      for (int r = 0; r < rows; r++) {
        int cols = 7 - (r * 2);
        int startX = 120 - ((cols * (blockSize + spacing)) / 2);
        for (int c = 0; c < cols; c++) {
          tft.fillRect(startX + c * (blockSize + spacing) - 2, mouthY + r * (blockSize + spacing), blockSize, blockSize, 0x07FF);
          tft.fillRect(startX + c * (blockSize + spacing), mouthY + r * (blockSize + spacing), blockSize, blockSize, ST77XX_WHITE);
        }
      }
    }

    delay(30);
  }
}

void display_begin() {
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(240, 280);
  tft.setSPISpeed(40000000);
  tft.setRotation(1);
  xTaskCreatePinnedToCore(displayTask, "Affichage", 10000, NULL, 1, &displayTaskHandle, 0);
}
