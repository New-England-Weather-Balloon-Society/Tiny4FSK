/*
oled.h, part of Tiny4FSK, for a high-altitude tracker.
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

#pragma once

#include <Arduino.h>
#include <Wire.h>

#define SSD1306_I2C_ADDRESS 0x3C

bool oled_begin(int16_t width, int16_t height, uint8_t i2c_addr = SSD1306_I2C_ADDRESS);
void oled_clearDisplay();
void oled_display();
void oled_drawPixel(int16_t x, int16_t y, uint16_t color);
void oled_setTextSize(uint8_t s);
void oled_setTextColor(uint16_t c);
void oled_setCursor(int16_t x, int16_t y);
void oled_print(const char* str);
void oled_print_diagnostic(const char* name, float value, int decimals);