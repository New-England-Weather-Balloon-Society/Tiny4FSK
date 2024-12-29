/*
crc_calc.h, part of Tiny4FSK, for a high-altitude tracker.
Copyright (C) 2024 Maxwell Kendall

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

// CRC calculation for FEC in the packet. This SHOULD NOT BE TOUCHED
// Only edit if you know what you are doing. If you accidentally deleted something (we all do that),
// then you can copy it back from GitHub.

#pragma once

#include <stdint.h>
#include <Arduino.h>

uint16_t crc_xmodem_update(uint16_t crc, uint8_t data);
unsigned int crc16(unsigned char *string, unsigned int len);