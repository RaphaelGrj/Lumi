#include "motion.h"
#include "pins.h"
#include "esp_camera.h"

static uint8_t *prev_frame = NULL;

void motion_begin() {
  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM; config.pin_d1 = Y3_GPIO_NUM; config.pin_d2 = Y4_GPIO_NUM; config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM; config.pin_d5 = Y7_GPIO_NUM; config.pin_d6 = Y8_GPIO_NUM; config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM; config.pin_pclk = PCLK_GPIO_NUM; config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM; config.pin_sscb_sda = SIOD_GPIO_NUM; config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM; config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE;
  config.frame_size = FRAMESIZE_QQVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[CAM] ERROR init 0x%x\n", err);
  } else {
    Serial.println("[CAM] OK");
  }

  prev_frame = (uint8_t *)malloc(160 * 120);
}

MotionResult motion_detect() {
  MotionResult result = {false, 0, 0};

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) return result;

  long sumX = 0, sumY = 0;
  int numMovedPixels = 0;

  if (prev_frame != NULL) {
    for (int y = 0; y < fb->height; y += 4) {
      for (int x = 0; x < fb->width; x += 4) {
        int idx = y * fb->width + x;
        if (abs(fb->buf[idx] - prev_frame[idx]) > 40) {
          sumX += x; sumY += y; numMovedPixels++;
        }
      }
    }
  }
  memcpy(prev_frame, fb->buf, fb->len);
  esp_camera_fb_return(fb);

  if (numMovedPixels > 10) {
    result.detected = true;
    result.gazeX = map(sumX / numMovedPixels, 160, 0, -35, 35);
    result.gazeY = map(sumY / numMovedPixels, 0, 120, -20, 20);
  }

  return result;
}
