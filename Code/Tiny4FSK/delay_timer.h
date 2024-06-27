// LOW-LEVEL DELAY FUNCTION
// These functions use the TC (timer counter) to achieve a more accurate delay.
#pragma once

#include <Arduino.h>

void TC3_Handler();
void setupTC3(uint16_t delay_ms);
void delayWithTC3(uint16_t delay_ms);