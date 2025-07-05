/*
utils.cpp, part of Tiny4FSK, for a high-altitude tracker.
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

#include "utils.h"

// Custom map function that supports floating-point mapping
double mapf(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Send out the Morse Code callsign
void sendCallsign()
{
#ifdef DEV_MODE
  Serial.println("Sending Morse Code Callsign!");
#endif
  si4063_set_frequency_offset(0);
  sendMorseString(CALLSIGN);
}

// Configure the Si4063 to user values
void configureSi4063()
{
  chip_parameters si_params;
  si_params.gpio0 = 0x00;
  si_params.gpio1 = 0x00;
  si_params.gpio2 = 0x00;
  si_params.gpio3 = 0x00;
  si_params.drive_strength = 0x00;
  si_params.clock = 26000000UL;

  radio_parameters rf_params;
  rf_params.frequency_hz = FSK_FREQ * 1000000;
  rf_params.power = OUTPUT_POWER;
  rf_params.type = SI4063_MODULATION_TYPE_CW;
  rf_params.offset = 0;
  rf_params.deviation_hz = 0x00;

  // Handle initialization error
  if (si4063_init(rf_params, si_params) != HAL_OK)
  {
#ifdef DEV_MODE
    Serial.println("Initialization Error!");
#endif
    while (1)
      ;
  }

  // Disable TX if in reset mode
  si4063_inhibit_tx();
}
