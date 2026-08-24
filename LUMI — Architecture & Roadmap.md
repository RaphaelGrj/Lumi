# LUMI — Architecture & Roadmap

> **Master Architecture Document**  
> This document is the single source of truth for understanding the Lumi project.  
> It is intended for humans and AI coding agents (Claude Code, Codex, Gemini CLI, etc.) before modifying the repository.

---

# 1. Project Identity

## Project name

**Lumi**

Repository:

<https://github.com/RaphaelGrj/Lumi-Project>

## What is Lumi?

Lumi is an **open-source AI desktop companion robot** designed to feel alive rather than behave like a simple connected gadget.

Its purpose is to become a small expressive robotic companion capable of:

- looking at people
- reacting emotionally
- speaking naturally with AI
- perceiving its environment
- interacting with Home Assistant
- evolving over time through personality and behavior

Lumi is part of the broader **R-BOT ecosystem**, where each robot is an independent project while sharing a common artistic identity.

The defining artistic direction is:

> **Cyber Glitch + Soft Companion**

Lumi should feel futuristic, expressive and adorable, with subtle cyan and magenta glitch effects around its eyes rather than aggressive cyberpunk visuals.

---

# 2. Core Vision

Lumi must never feel like a collection of disconnected modules.

The project philosophy is that **every sensor influences personality**.

Example:

| Event | Emotional reaction |
|---|---|
| Nobody detected for a long time | Boredom |
| Person appears | Happy / curious |
| User speaks | Listening expression |
| Strong shake | Dizzy / annoyed |
| Cold temperature | Shivering |
| Hot environment | Sweating / tired |
| Gentle tap | Cancel current action |
| Conversation | Personality evolves |

The objective is **organic behavior**, not scripted animations.

Every subsystem contributes to one global internal state.

---

# 3. Design Principles

## Golden Rules

1. Lumi is an emotional robot first.
2. Hardware and software remain modular.
3. Every feature must communicate through clear interfaces.
4. Display rendering must never block AI or sensors.
5. Audio must remain responsive.
6. Wi-Fi credentials must never be hardcoded.
7. Configuration belongs outside business logic whenever possible.
8. Future versions must remain backward understandable.

---

# 4. Current Hardware Architecture

## Main architecture

```text
                         USER
                           │
                           ▼
                    ┌─────────────┐
                    │ ESP32-CAM   │
                    │ Vision Core │
                    └──────┬──────┘
                           │ UART
                           │
                           ▼
┌──────────────────────────────────────────┐
│               ESP32-S3                  │
│                                          │
│              LUMI BRAIN                 │
│                                          │
│  State Machine                          │
│  Personality                            │
│  AI Client                              │
│  Audio Manager                          │
│  Sensor Manager                         │
│  Display Manager                        │
│  Home Assistant                         │
│                                          │
└──────┬──────────┬───────────┬───────────┘
       │          │           │
       │          │           │
       ▼          ▼           ▼
   ST7789      BME280      MPU6050
    Eyes     Temperature    Motion
                Climate      Touch
       │
       ▼
 MAX98357A
       │
       ▼
   Speaker

       ▲
       │
   INMP441
   Microphone
```

The ESP32-S3 is the central brain.

The ESP32-CAM is intentionally separated so computer vision does not overload the main controller.

---

# 5. Hardware Inventory

## Mandatory Hardware

| Component | Role |
|---|---|
| ESP32-S3 N16R8 | Main brain |
| ESP32-CAM | Vision processor |
| ST7789 240×280 TFT | Eye display |
| BME280 | Temperature / humidity / pressure |
| MPU6050 | Accelerometer / gyroscope |
| INMP441 | I2S microphone |
| MAX98357A | I2S speaker amplifier |
| Speaker | Voice output |

---

# 6. ESP32-S3 Responsibilities

The ESP32-S3 owns all high-level behavior.

It must manage:

- emotional state
- eye animations
- audio capture
- voice playback
- Gemini communication
- Wi-Fi portal
- Home Assistant
- sensor interpretation
- camera communication
- personality persistence
- future memory system

It must **never perform heavy image processing**.

That belongs to the ESP32-CAM.

---

# 7. ESP32-CAM Responsibilities

The ESP32-CAM is dedicated exclusively to visual perception.

Its responsibilities are intentionally limited.

## Current

- motion detection
- person presence detection
- gaze position estimation

## Future

- face recognition
- animal recognition
- creator recognition
- multiple person tracking
- gesture recognition

Communication with the ESP32-S3 is performed through UART.

Example protocol:

```text
T,x,y
P,1
E,id
```

Where:

| Prefix | Meaning |
|---|---|
| T | Target gaze coordinates |
| P | Presence detected |
| E | Entity identifier |

Example:

```text
P,1
T,35,-18
E,1
```

The protocol should remain lightweight and human-readable.

---

# 8. Software Architecture

Lumi software follows a modular architecture.

```text
                    ┌──────────────────┐
                    │     setup()      │
                    └────────┬─────────┘
                             │
          ┌──────────────────┼──────────────────┐
          │                  │                  │
          ▼                  ▼                  ▼
   Display Manager    Sensor Manager      Audio Manager
          │                  │                  │
          ▼                  ▼                  ▼
     Eye Task         BME280 MPU6050      Mic + Speaker
          │                  │                  │
          └──────────────────┼──────────────────┘
                             │
                             ▼
                    State Machine
                             │
                             ▼
                    Behavior Engine
                             │
          ┌──────────────────┼──────────────────┐
          │                  │                  │
          ▼                  ▼                  ▼
      Gemini Client      Home Assistant     Camera UART
```

No subsystem should directly modify another subsystem's internal rendering or state.

Everything should converge toward the **State Machine**.

---

# 9. FreeRTOS Architecture

Lumi runs multiple concurrent tasks.

The objective is responsiveness.

## Recommended Tasks

### TaskDisplay

Responsible only for rendering.

Responsibilities:

- draw eyes
- blink
- gaze interpolation
- expressions
- glitch effects
- Wi-Fi icon
- overlays

Priority:

High

Core:

0

Must never perform network operations.

---

### TaskSensors

Responsible for reading physical sensors.

Reads:

- BME280
- MPU6050
- future touch sensors

Outputs normalized events.

Example:

```text
EVENT_SHAKE
EVENT_TAP
EVENT_HOT
EVENT_COLD
```

---

### TaskCamera

Responsible for UART communication.

Responsibilities:

- receive camera messages
- validate protocol
- update gaze target
- update presence
- update entity ID

Must never draw directly.

---

### TaskAudio

Responsible for voice.

Pipeline:

```text
Microphone
    │
    ▼
Voice Detection
    │
    ▼
Recording
    │
    ▼
Gemini
    │
    ▼
Text Response
    │
    ▼
TTS
    │
    ▼
Speaker
```

---

### TaskBehavior

This is Lumi's heart.

It calculates:

- mood
- attention
- boredom
- happiness
- irritation
- sleepiness
- curiosity

Everything eventually influences this task.

---

# 10. Display Architecture

The ST7789 display represents Lumi's personality.

Important rule:

> Rendering belongs exclusively to the display task.

Other modules communicate desired states.

Example:

```cpp
desiredExpression = EXPR_HAPPY;
gazeTargetX = 0.35;
gazeTargetY = -0.18;
```

The display task interpolates visually.

Never instantly jump unless intentionally required.

---

## Eye System

Lumi has one physical display representing two eyes.

Coordinate concept:

```text
┌──────────────────────────┐
│       LEFT    RIGHT      │
│                          │
│        ◉        ◉        │
│                          │
└──────────────────────────┘
```

Eyes should support:

- blink
- half blink
- surprise
- sleepy
- angry
- happy
- listening
- confused
- dizzy
- glitch

---

# 11. Emotional State Machine

The State Machine must become the central behavioral system.

## Primary moods

```text
IDLE
CURIOUS
HAPPY
LISTENING
THINKING
TALKING
BORED
SLEEPY
ANNOYED
DIZZY
SURPRISED
```

These are not animations.

They are behavioral states.

Each state influences:

- eye shape
- blink speed
- gaze behavior
- idle movement
- voice tone
- reactions

---

## Example transition

```text
No presence
      │
      ▼
    IDLE
      │
      │ 30 min
      ▼
    BORED
      │
Person appears
      │
      ▼
   CURIOUS
      │
User speaks
      │
      ▼
  LISTENING
      │
Gemini processing
      │
      ▼
  THINKING
      │
Response ready
      │
      ▼
   TALKING
      │
      ▼
   CURIOUS
```

---

# 12. Mood Engine

Mood should not be binary.

Instead use continuous variables.

Example concept:

```cpp
struct Mood
{
    float happiness;
    float boredom;
    float curiosity;
    float energy;
    float irritation;
};
```

Sensors modify values gradually.

Example:

| Event | Effect |
|---|---|
| Presence | + curiosity |
| Conversation | + happiness |
| Long silence | + boredom |
| Shake | + irritation |
| Night | - energy |
| Cold | comfort discomfort |
| Hot | fatigue |

The displayed expression is generated from these values.

---

# 13. AI Architecture

Gemini provides conversational intelligence.

Gemini is **not responsible for controlling hardware directly**.

Gemini only returns semantic information.

Example:

```text
User:
"Salut Lumi !"

Gemini:
"Salut ! Tu m'as manqué aujourd'hui."
```

The Behavior Engine decides:

- expression
- gaze
- animation
- voice playback

AI must never directly manipulate GPIO.

---

# 14. Voice Interaction

Voice interaction must feel immediate.

## Wake behavior

Current concept:

```text
Presence detected
        │
        ▼
User speaks
        │
        ▼
Listening eyes
        │
        ▼
Audio recording
        │
        ▼
Gemini
        │
        ▼
Thinking animation
        │
        ▼
Speech output
```

Future improvements:

- wake word
- interruption detection
- natural conversation memory
- multiple speakers

---

# 15. Sensor Architecture

## BME280

Purpose:

Environmental awareness.

Inputs:

- temperature
- humidity
- pressure

Outputs examples:

```text
TEMP_COLD
TEMP_COMFORT
TEMP_HOT
```

Possible expressions:

Cold:

- trembling pupils
- cyan frost
- small icicles

Hot:

- sweat drops
- heavy eyelids
- slower blink

---

## MPU6050

Purpose:

Body awareness.

Detect:

- shake
- rotation
- impact
- tap

Special feature:

### Tap to Cancel

A gentle tap interrupts:

- listening
- AI request
- voice playback

This should always remain responsive.

---

# 16. Camera Communication

UART is the only communication bridge between the two ESP32 boards.

## Hardware

```text
ESP32-S3 TX ─────► ESP32-CAM RX

ESP32-S3 RX ◄───── ESP32-CAM TX

GND ────────────── GND
```

Never connect TX to TX.

Ground is mandatory.

---

## Protocol rules

Messages must:

- end with newline
- remain ASCII
- remain short
- remain human-readable

Valid examples:

```text
P,1
P,0

T,25,-14

E,1
```

Future extension:

```text
F,name
A,cat
G,wave
```

Protocol must remain backward compatible.

---

# 17. Wi-Fi Architecture

Lumi uses a captive portal.

Objectives:

- no hardcoded credentials
- easy first boot
- safe open-source repository
- API key configuration

Boot sequence:

```text
Power ON
   │
   ▼
Display starts
   │
   ▼
Sensors initialize
   │
   ▼
Audio initialize
   │
   ▼
Wi-Fi Portal
   │
   ├── First boot → configuration page
   │
   └── Existing config → automatic connection
```

The display must already be alive before networking.

This is important for diagnostics.

---

# 18. Home Assistant Integration

Home Assistant transforms Lumi into a physical interface for the smart home.

Examples:

User says:

> Turn on the living room.

Flow:

```text
Voice
  │
  ▼
Gemini
  │
Intent extraction
  │
  ▼
Home Assistant
  │
  ▼
Light ON
```

Future interactions:

- notifications
- doorbell reactions
- weather awareness
- alarm reactions
- room sensors
- automation triggers

Architecture should remain API based.

No hardcoded automation logic.

---

# 19. Personality System

Personality is a major future pillar.

The objective is that Lumi gradually develops recognizable behavior.

Possible persistent traits:

```text
Humor
Curiosity
Attachment
Energy
Patience
Playfulness
```

These values influence responses rather than replacing Gemini.

Example:

High curiosity:

> "Oh ! Qu'est-ce que tu fabriques aujourd'hui ?"

High boredom:

> "Tu m'abandonnes encore devant ton écran..."

Persistence should eventually be stored locally.

---

# 20. Repository Architecture

Target repository structure.

```text
Lumi-Project/

│
├── Code/
│   │
│   ├── Lumi_S3/
│   │   ├── main.cpp
│   │   ├── display.cpp
│   │   ├── display.h
│   │   ├── behavior.cpp
│   │   ├── behavior.h
│   │   ├── sensors.cpp
│   │   ├── sensors.h
│   │   ├── audio.cpp
│   │   ├── audio.h
│   │   ├── camera_link.cpp
│   │   ├── camera_link.h
│   │   ├── wifi.cpp
│   │   ├── wifi.h
│   │   ├── gemini.cpp
│   │   └── gemini.h
│   │
│   └── Lumi_CAM/
│       ├── main.cpp
│       ├── motion.cpp
│       ├── tracking.cpp
│       └── protocol.cpp
│
├── assets/
│
├── images/
│
├── Hardware/
│
├── STL/
│
├── Docs/
│
├── ARCHITECTURE_AND_ROADMAP.md
│
├── README.md
│
└── LICENSE
```

This structure is the target.

The current repository may temporarily differ while development is ongoing.

---

# 21. Coding Rules

Every contributor and AI agent must respect these rules.

## Never

- hardcode Wi-Fi credentials
- hardcode Gemini API keys
- block the display task
- perform long delays inside rendering
- access display from multiple tasks
- directly couple camera logic with UI rendering
- mix business logic and GPIO everywhere

---

## Always

- keep modules independent
- document public interfaces
- use descriptive names
- preserve UART protocol compatibility
- maintain backward compatibility where possible
- prefer state driven behavior
- keep diagnostics available through Serial

---

# 22. Debug Philosophy

Every subsystem must be independently testable.

Boot diagnostics should clearly print:

```text
[BOOT] ESP32-S3 started

[TFT] OK

[BME280] OK

[MPU6050] OK

[MIC] OK

[SPEAKER] OK

[CAM] UART ready

[WIFI] Connected

[GEMINI] Ready
```

If something fails:

```text
[TFT] ERROR

[CAM] TIMEOUT

[BME280] NOT FOUND
```

Never silently fail.

Diagnostics are essential because Lumi contains multiple interconnected systems.

---

# 23. Development Status

Legend:

- ✅ Implemented
- 🟡 In progress
- 🔴 Not implemented
- ⚪ Planned

## Current V1

| Feature | Status |
|---|---|
| ESP32-S3 firmware | 🟡 |
| Wi-Fi captive portal | 🟡 |
| Gemini API connection | 🟡 |
| ST7789 eye display | 🟡 |
| Eye animation system | 🟡 |
| ESP32-CAM communication | 🟡 |
| Motion tracking | 🟡 |
| Microphone capture | 🟡 |
| Speaker output | 🟡 |
| BME280 integration | 🟡 |
| MPU6050 integration | 🟡 |
| Home Assistant | 🔴 |
| Persistent personality | 🔴 |
| Face recognition | 🔴 |
| Animal recognition | 🔴 |

---

# 24. Roadmap

# Phase 0 — Foundation

Goal:

Create a stable hardware platform.

Tasks:

- [ ] Validate ESP32-S3 boot
- [ ] Validate ST7789 display
- [ ] Validate FreeRTOS display task
- [ ] Validate UART camera communication
- [ ] Validate microphone
- [ ] Validate speaker
- [ ] Validate BME280
- [ ] Validate MPU6050

Exit condition:

Every hardware component independently works.

---

# Phase 1 — Visual Life

Goal:

Make Lumi visually alive.

Tasks:

- [ ] Smooth gaze interpolation
- [ ] Blink engine
- [ ] Idle eye movement
- [ ] Happy expression
- [ ] Curious expression
- [ ] Listening expression
- [ ] Thinking animation
- [ ] Talking animation
- [ ] Glitch effects
- [ ] Wi-Fi indicator

Exit condition:

Lumi looks alive without AI.

---

# Phase 2 — Physical Perception

Goal:

Connect sensors to emotions.

Tasks:

- [ ] Presence events
- [ ] Motion tracking
- [ ] Shake detection
- [ ] Tap cancel
- [ ] Temperature reactions
- [ ] Humidity reactions
- [ ] Climate mood influence

Exit condition:

Sensors visibly affect behavior.

---

# Phase 3 — Voice

Goal:

Natural spoken interaction.

Tasks:

- [ ] Voice detection
- [ ] Audio recording
- [ ] Gemini request
- [ ] Response parsing
- [ ] TTS playback
- [ ] Listening interruption
- [ ] Tap interruption

Exit condition:

Full voice conversation works reliably.

---

# Phase 4 — Intelligence

Goal:

Create personality.

Tasks:

- [ ] Mood variables
- [ ] Emotional transitions
- [ ] Long-term memory
- [ ] Personality traits
- [ ] Contextual conversations
- [ ] Creator recognition concept

Exit condition:

Lumi develops consistent behavior.

---

# Phase 5 — Vision

Goal:

Advanced perception.

Tasks:

- [ ] Face recognition
- [ ] Animal recognition
- [ ] Multi-person tracking
- [ ] Entity identification
- [ ] Gesture recognition

Exit condition:

Lumi recognizes its environment.

---

# Phase 6 — Smart Home

Goal:

Become a physical Home Assistant companion.

Tasks:

- [ ] Connect HA API
- [ ] Device control
- [ ] Automation triggers
- [ ] Notifications
- [ ] Weather integration
- [ ] Smart routines

Exit condition:

Lumi naturally interacts with the connected home.

---

# Phase 7 — MK2 Evolution

Future hardware revision.

Potential improvements:

- custom PCB
- improved audio
- better microphone array
- improved camera
- redesigned mechanics
- battery system
- magnetic dock
- richer display technology

This phase must remain compatible with the V1 software philosophy whenever possible.

---

# 25. AI Agent Working Contract

This section is specifically written for Claude Code, Codex and similar coding agents.

Before editing any file:

1. Read this document entirely.
2. Inspect the current repository structure.
3. Identify existing architecture before modifying it.
4. Never assume a module exists if it is not present.
5. Preserve working functionality.
6. Prefer incremental modifications.
7. Explain architectural impact before major refactors.
8. Update this document whenever architecture meaningfully changes.

When implementing a feature:

- determine affected modules
- avoid cross-module spaghetti dependencies
- keep hardware abstraction clear
- preserve debug output
- preserve UART protocol
- maintain display ownership by one task only

When unsure:

> Prefer extending the existing architecture rather than replacing it.

---

# 26. Definition of Done

A feature is considered complete only when:

- [ ] Code compiles successfully.
- [ ] Existing features remain functional.
- [ ] Serial diagnostics exist.
- [ ] No blocking behavior is introduced.
- [ ] Architecture remains modular.
- [ ] Documentation is updated if needed.
- [ ] Hardware test procedure exists.
- [ ] Failure cases are handled gracefully.

---

# 27. Long Term Vision

Lumi is not intended to become merely a voice assistant inside a plastic shell.

The ultimate objective is to create a believable robotic companion whose personality emerges from the interaction between:

- vision
- voice
- emotions
- environment
- movement
- connected home
- artificial intelligence

Every future feature should reinforce one idea:

> **Lumi should feel present.**

Not because it performs many functions, but because all of its systems cooperate to create the illusion of a small living companion sitting on a desk.

---

# 28. Architecture Decision Records

Major architectural decisions should be appended below.

## ADR-001 — Dual ESP32 Architecture

Decision:

Use an ESP32-S3 as the main brain and an ESP32-CAM as a dedicated vision processor.

Reason:

Separates computer vision from emotional behavior and improves responsiveness.

Status:

Accepted.

---

## ADR-002 — FreeRTOS Display Ownership

Decision:

Only one FreeRTOS task owns the TFT display.

Reason:

Prevents concurrent SPI drawing conflicts and keeps animations smooth.

Status:

Accepted.

---

## ADR-003 — Human Readable UART Protocol

Decision:

Camera communication uses short ASCII messages.

Reason:

Easy debugging through Serial Monitor and future extensibility.

Status:

Accepted.

---

## ADR-004 — State Driven Personality

Decision:

Sensors never directly trigger animations.

They modify emotional state, then the behavior engine determines visual output.

Reason:

Creates organic and scalable behavior.

Status:

Accepted.

---

# End of Master Document

Whenever the architecture evolves, this file must evolve with it.

It is the canonical onboarding document for Lumi developers and AI coding agents.
