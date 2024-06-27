#include "voltage.h"
#include <Arduino.h>

double readVoltage() {
  int rawVolt = analogRead(A0);
  return rawVolt * (3.30 / 1023.00);
}