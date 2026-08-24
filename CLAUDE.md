# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Lumi is an open-source AI desktop companion robot (part of the R-BOT ecosystem), built on two PlatformIO/Arduino targets that communicate over UART:

- **Brain — ESP32-S3 (N16R8)**: `src/brain/` (modular, see below). Owns the TFT eye display, Wi-Fi captive portal, wake-word listening, Wit.ai STT, Gemini API calls, TTS playback, BME280/MPU6050 sensors.
- **Vision — ESP32-CAM (AI-Thinker)**: `src/vision/` (modular, see below). Dedicated to motion/gaze detection via frame-difference on a grayscale QQVGA feed, reported to the brain over UART.

Full product vision, hardware inventory, target software architecture, FreeRTOS task design, state machine, and roadmap live in `LUMI — Architecture & Roadmap.md` — **read it before making non-trivial changes**, it is the canonical spec this repo is converging toward. Session-to-session progress, decisions, and open problems are tracked in `PROGRESS.md` — **read it first, and update it before ending a session** (see below).

## Current state of the code (important — read before assuming structure)

As of 2026-08-24 both firmwares are modularized, matching the file names from the architecture doc §20 (adapted to the existing `src/brain/` / `src/vision/` folders — see below for why). There is still no FreeRTOS multi-task split beyond the brain's single display task on core 0 (§9's TaskSensors/TaskCamera/TaskAudio/TaskBehavior separation is deliberately deferred — see `PROGRESS.md` 2026-08-24 entry for why), and the state machine is a plain 5-value enum (`EmotionState` in `src/brain/state.h`), not the full continuous Mood Engine of §11-12.

**Hardware wiring is frozen for everything already built** (TFT, I2S mic, I2S amp, I2C, boot button, camera pins below) — do not change those without the user confirming a rewire. The brain↔camera UART link is the one exception: it was never physically wired, so the pins in `pins.h` on both sides are a **proposal pending confirmation before soldering** (marked as such in the files).

### Brain (ESP32-S3) — `src/brain/`, pins in `pins.h`
- TFT (ST7789, 240×280 SPI): MOSI=11, SCLK=12, CS=10, DC=13, RST=14 — owned by `display.cpp`
- I2S mic (INMP441, I2S_NUM_1 RX): SCK=4, WS=5, SD=6 — owned by `audio.cpp`
- I2S amp (MAX98357A, TX via Audio lib): BCLK=15, LRC=16, DIN=17 — owned by `audio.cpp`
- I2C (BME280 + MPU6050): SDA=8, SCL=9 — owned by `sensors.cpp`
- Boot button (force config portal): GPIO0 — owned by `wifi.cpp`
- UART to camera (`Serial1`): TX=GPIO43, RX=GPIO44 — **proposed**, owned by `camera_link.cpp`. Free because `ARDUINO_USB_CDC_ON_BOOT=1` routes `Serial` through native USB, leaving UART0's default pins open.

Modules: `state.h` (enum + shared `currentState`), `behavior.cpp` (only writer of `currentState`), `display.cpp` (eye rendering task), `sensors.cpp`, `audio.cpp` (mic, Wit.ai STT, TTS playback, wake-word check against the `WAKE_WORD` build flag), `gemini.cpp`, `wifi.cpp` (portal + secrets), `camera_link.cpp` (UART parsing), `main.cpp` (orchestration only — read this first to see the control flow).

### Vision (ESP32-CAM, AI-Thinker) — `src/vision/`, pins in `pins.h`
Standard AI-Thinker camera pinout (PWDN=32, XCLK=0, SIOD=26, SIOC=27, Y2-Y9/VSYNC/HREF/PCLK). Modules: `motion.cpp` (camera init + frame-diff motion detection), `protocol.cpp` (UART protocol out), `main.cpp` (orchestration).

**Important constraint:** the AI-Thinker board exposes only one UART (GPIO1 TX / GPIO3 RX), already used for flashing and for `Serial`. There is no second UART to dedicate to the brain link — `protocol.cpp` sends protocol lines over the same `Serial` used for boot diagnostics, exactly like the original code did. This means the FTDI programmer and the S3 link cannot be connected at the same time; unplug one before using the other.

### UART protocol (brain ↔ camera)
Vision emits `T,<x>,<y>` (gaze offset, only when motion is detected) and `P,1`/`P,0` (presence, latched with a ~2.5s timeout — see `protocol_reportPresence` in `src/vision/protocol.cpp`), matching the architecture doc §7/§16. The brain's `camera_link.cpp` parses these and exposes `camera_hasPresence()`/`camera_gazeX()`/`camera_gazeY()`, but **nothing consumes them yet** — eye movement in `display.cpp` is still the autonomous random idle motion. Wiring camera gaze into eye tracking is open work (see `PROGRESS.md`). No `E,id` messages are sent — no recognition capability exists yet, so nothing should claim to identify anyone.

### Divergence from the architecture doc's target layout
The doc (§20) proposes `Code/Lumi_S3/` / `Code/Lumi_CAM/` folders. This repo keeps `src/brain/` / `src/vision/` instead, because `platformio.ini`'s `build_src_filter` already splits the build by those folder names — renaming would require touching `platformio.ini` for no behavioral benefit. Only the *file names inside* those folders follow the doc's target (`display.cpp`, `behavior.cpp`, etc.).

### Secrets
No hardcoded Wi-Fi/API credentials — `wifi.cpp` uses `WiFiManager` captive portal (`LUMI_CONFIGURATION`) to collect the Gemini API key and Wit.ai token on first boot, persisted via `Preferences` (NVS), exposed to other modules via `wifi_getGeminiApiKey()`/`wifi_getWitAiToken()`. Keep it this way; never hardcode credentials into source.

## Build / upload / monitor

Two PlatformIO environments, split by `build_src_filter` so each board only compiles its own folder (`src/brain/` vs `src/vision/`):

```bash
pio run -e lumi_brain_s3                  # compile brain only
pio run -e lumi_brain_s3 -t upload        # flash brain (update upload_port in platformio.ini first)
pio device monitor -e lumi_brain_s3       # serial monitor, 115200 baud

pio run -e lumi_vision_cam                # compile vision only
pio run -e lumi_vision_cam -t upload      # flash vision (upload_port is commented out — set it before use)
pio device monitor -e lumi_vision_cam
```

There is no test suite configured (`test/` is the default empty PlatformIO stub) and no linter — verification is "does it compile" plus manual hardware testing.

## Coding rules (from the architecture doc §21 — apply these when touching either firmware)

**Never:**
- Hardcode Wi-Fi credentials or API keys (see Secrets above)
- Block the display task, or perform long delays inside rendering
- Let more than one task/module touch the display
- Couple camera logic directly to UI rendering

**Always:**
- Keep the brain/vision split intact — the brain never does heavy image processing, the camera never renders UI
- Preserve the UART protocol's ASCII, newline-terminated, human-readable format when extending it
- Preserve `Serial` boot diagnostics (`[BOOT]`, `[TFT] OK`, `[CAM] TIMEOUT`, etc. — see doc §22) when adding subsystem init code; fail loudly, never silently
- Keep each module's `_begin()`/`_update()` pattern and single responsibility — put new logic in the module that owns the relevant hardware/concern rather than back into `main.cpp`

## Session workflow

Update `PROGRESS.md` at the end of a working session (or when a problem gets solved) with: what changed, what was tried and failed and why, and what's next. This is the primary way context carries over between sessions — check it first, keep it current.
