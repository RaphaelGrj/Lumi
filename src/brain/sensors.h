#pragma once

void sensors_begin();
void sensors_update(); // à appeler périodiquement depuis loop()

float sensors_getTemperature();
bool sensors_isBmeActive();
bool sensors_isMpuActive();
