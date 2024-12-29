/*
4fsk_mod.cpp, part of Tiny4FSK, for a high-altitude tracker.
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

// MFSK Modulation
#include "4fsk_mod.h"

void fsk4_writebyte(uint8_t b) {
  // Send symbols MSB first.
  for (int k = 0; k < 4; k++) {
    // Extract 4FSK symbol (2 bits)
    uint8_t symbol = (b & 0xC0) >> 6;
    // Modulate
    si4063_set_frequency_offset(22 * symbol);
    delay(10);
    // Shift to next symbol.
    b = b << 2;
  }
}

void fsk4_write(char *buff, size_t len) {
  for (int i = 0; i < len; i++) {
    fsk4_writebyte(buff[i]);
  }
}
void fsk4_preamble(uint8_t len) {
  int k;
  for (k = 0; k < len; k++) {
    fsk4_writebyte(0x1B);
  }
}

void fsk4_idle() {
  si4063_set_frequency_offset(0);
}