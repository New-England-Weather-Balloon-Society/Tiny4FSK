#include <Arduino.h>

#ifndef delay_timer_guard
#define delay_timer_guard

void TC3_Handler();
void setupTC3(uint16_t delay_ms);
void delayWithTC3(uint16_t delay_ms);

#endif