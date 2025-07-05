/*
voltage.cpp, part of Tiny4FSK, for a high-altitude tracker.
Copyright (C) 2025 Maxwell Kendall

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

double readVoltage() {
  const int numReadings = 3; // Number of readings to average
  double totalVoltage = 0.0;

  for (int i = 0; i < numReadings; i++) {
    int rawValue = analogRead(VOLTMETER_PIN); // Read ADC value
    double voltage = rawValue * (3.3 / 1023.0) * 2; // Convert to voltage
    totalVoltage += voltage; // Accumulate the voltage
    delay(5); // Small delay for noise reduction (optional)
  }

  return totalVoltage / numReadings; // Return the average voltage
}