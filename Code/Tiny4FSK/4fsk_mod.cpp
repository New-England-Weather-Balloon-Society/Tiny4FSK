// MFSK Modulation
#include "4fsk_mod.h"

void fsk4_writebyte(uint8_t b) {
  // Send symbols MSB first.
  for (int k = 0; k < 4; k++) {
    // Extract 4FSK symbol (2 bits)
    uint8_t symbol = (b & 0xC0) >> 6;
    // Modulate
    si4063_set_frequency_offset(22 * symbol);
    //delayWithTC3(10);
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