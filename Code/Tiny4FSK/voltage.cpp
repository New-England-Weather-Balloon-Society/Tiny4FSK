#include "voltage.h"
#include <Arduino.h>

double readVoltage() {
  float firstVolt = (analogRead(VOLTMETER_PIN) * 2) * (3.3 / 1023.0);
  delay(10);
  float secondVolt = (analogRead(VOLTMETER_PIN) * 2) * (3.3 / 1023.0);
  delay(5);
  return ((firstVolt + secondVolt + (analogRead(VOLTMETER_PIN) * 2) * (3.3 / 1023.0)) / 3);
}