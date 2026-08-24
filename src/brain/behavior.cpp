#include "behavior.h"
#include "state.h"

volatile EmotionState currentState = EmotionState::BOOT_GLITCH;

void behavior_setBootGlitch() { currentState = EmotionState::BOOT_GLITCH; }
void behavior_setIdle()       { currentState = EmotionState::IDLE; }
void behavior_setListening()  { currentState = EmotionState::LISTENING; }
void behavior_setTalking()    { currentState = EmotionState::TALKING; }
void behavior_setWifiPortal() { currentState = EmotionState::WIFI_PORTAL; }

bool behavior_isTalking() { return currentState == EmotionState::TALKING; }
