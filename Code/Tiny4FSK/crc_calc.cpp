// CRC calculation for FEC in the packet. This SHOULD NOT BE TOUCHED!!!
// Only edit if you know what you are doing. If you accidentally deleted something (we all do that),
// then you can copy it back from GitHub.

#include "crc_calc.h"
#include <Arduino.h>

uint16_t crc_xmodem_update(uint16_t crc, uint8_t data) {
  int i;
  crc = crc ^ ((uint16_t)data << 8);
  for (i = 0; i < 8; i++) {
    if (crc & 0x8000)
      crc = (crc << 1) ^ 0x1021;
    else
      crc <<= 1;
  }
  return crc;
}

unsigned int crc16(unsigned char *string, unsigned int len) {
  unsigned int i;
  unsigned int crc;
  crc = 0xFFFF;
  for (i = 0; i < len; i++) {
    crc = crc_xmodem_update(crc, (uint8_t)string[i]);
  }
  return crc;
}