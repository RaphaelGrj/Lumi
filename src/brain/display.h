#pragma once

// Initialise le TFT et démarre la tâche FreeRTOS d'affichage (core 0).
// Ne doit jamais être appelée plus d'une fois.
void display_begin();
