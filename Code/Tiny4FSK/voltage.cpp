/*
voltage.cpp, part of Tiny4FSK, for a high-altitude tracker.
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

#include "voltage.h"
#include <Arduino.h>

double readVoltage() {
  float firstVolt = (analogRead(VOLTMETER_PIN) * 2) * (3.3 / 1023.0);
  delay(10);
  float secondVolt = (analogRead(VOLTMETER_PIN) * 2) * (3.3 / 1023.0);
  delay(5);
  return ((firstVolt + secondVolt + (analogRead(VOLTMETER_PIN) * 2) * (3.3 / 1023.0)) / 3);
}