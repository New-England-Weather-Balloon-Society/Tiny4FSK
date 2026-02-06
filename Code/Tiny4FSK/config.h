/*
config.h, part of Tiny4FSK, for a high-altitude tracker.
Copyright (C) 2026 Maxwell Kendall

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

// TINY4FSK CONFIG FILE
// Please read comments for each section, even though some settings are self-explanatory.
// Feel free to add an issue or PR on GitHub for any questions or bugs.

#pragma once

// **********************
// || Tracker Settings ||
// **********************

// Horus Binary v2 ID. Obtain a valid ID from https://github.com/projecthorus/horusdemodlib/
// Create an issue requesting a v2 ID with your callsign. Example Issue:
/*
Hello!
Could I please get a Horus v2 ID for N0CALL? Thanks!

-Name Here
*/

// Replace with the assigned Horus v2 ID
#define HORUS_ID 380

// Amateur radio license required! Give callsign here.
#define CALLSIGN "W0MXX"

// Transmits callsign in CW to stay legal. Keep at 20 or below.
#define CALLSIGN_WPM 20

// Interval to send the CW in milliseconds.
#define CALLSIGN_INTERVAL 600000

// FSK Center Frequency in MHz. Ensure SDR is tuned to this frequency.
#define FSK_FREQ 432.634

// Baud Rate of FSK Packet. No need to change.
#define FSK_BAUD 100

// Spacing of FSK peaks. Adjust in the decoding program (e.g., Horus GUI, HorusDemodLib).
#define FSK_SPACING 270

// Delay between each packet, in milliseconds.
#define PACKET_INTERVAL 1000

// Si4063 Transmit Power Level
#define OUTPUT_POWER 127

// If the GPS position seems to be a bad position (altitude less than zero, GPS reports bad fix),
// then transmit all zeros.
#define FLAG_BAD_PACKET

// ****************************
// || General Board Settings ||
// ****************************

// Enable status mode LEDs for information on GPS initialization and issues.
#define STATUS_LED

// Enable Serial Prints for debugging and sleep modes for MCU.
// MCU sleep mode disabled debug messages, so disable for MCU sleep.
// Disable for flights to conserve power.
#define DEV_MODE

// *********************
// || Pin Definitions ||
// *********************

// You most likely do not need to touch these if you are using the PCB.
// Mostly for individual development or shields.

// Si4063 Pins. These are in junction with normal SPI pins (MISO, MOSI, SCK).
#define NSEL_PIN 10
#define SDN_PIN 11

// GPS External interrupt Pin. In junction with UART pins.
#define EXTINT_GPS 8

// Status LED Pins
#define ERROR_LED 5
#define SUCCESS_LED 4

// Voltage Divider Pin
#define VOLTMETER_PIN A0

// SD Card chip select pin
#define SD_CS 38 // Default on shield

// *****************
// || Custom Pins ||
// *****************

// If you have custom pins, define them here.
// For example, if you have a custom sensor on pin 6:
//#define CUSTOM_SENSOR 6

//*****************************
// || Tiny4FSK Shield Config ||
//*****************************

#define BME_ADDRESS 0x77
#define IMU_ADDRESS 0x68