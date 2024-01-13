// CRC calculation for FEC in the packet. This SHOULD NOT BE TOUCHED
// Only edit if you know what you are doing. If you accidentally deleted something (we all do that),
// then you can copy it back from GitHub.

#ifndef CRC_CALC_H
#define CRC_CALC_H

#include <stdint.h>

uint16_t crc_xmodem_update(uint16_t crc, uint8_t data);
unsigned int crc16(unsigned char *string, unsigned int len);

#endif // CRC_CALC_H
