// Morse Code callsign sending routines

#pragma once

#include "si4063.h"
#include "config.h"
#include <Arduino.h>

// Calculate the durations based on WPM
#define DOT_DURATION (1200 / CALLSIGN_WPM)
#define DASH_DURATION (3 * DOT_DURATION)
#define SPACE_DURATION DOT_DURATION
#define LETTER_SPACE_DURATION (3 * DOT_DURATION)
#define WORD_SPACE_DURATION (7 * DOT_DURATION)

void sendMorseChar(char c);
void sendMorseString(const char *s);