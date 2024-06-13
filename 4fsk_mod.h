#include <Arduino.h>
#include "si4063.h"
#include "delay_timer.h"
#include <SPI.h>

void fsk4_writebyte(uint8_t b);
void fsk4_write(char *buff, size_t len);
void fsk4_preamble(uint8_t len);
void fsk4_idle();
void fsk4_test_packet(int buff);