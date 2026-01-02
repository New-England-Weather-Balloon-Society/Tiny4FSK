/*
delay_timer.h, part of Tiny4FSK, for a high-altitude tracker.
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

// LOW-LEVEL DELAY FUNCTION
// These functions use the TC (timer counter) to achieve a more accurate delay.
#pragma once

#include <Arduino.h>

void TC3_Handler();
void setupTC3(uint16_t delay_ms);
void delayWithTC3(uint16_t delay_ms);