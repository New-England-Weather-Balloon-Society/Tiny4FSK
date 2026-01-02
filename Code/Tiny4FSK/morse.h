/*
morse.h, part of Tiny4FSK, for a high-altitude tracker.
Copyright (C) 2026 Maxwell Kendall

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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