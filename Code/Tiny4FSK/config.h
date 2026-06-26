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

// Amateur radio license is required! Give callsign here.
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
#define PACKET_INTERVAL 1000 // For 90% of use cases, try to keep this over 10 

// Si4063 Transmit Power Level
#define OUTPUT_POWER 127

// If the GPS position seems to be a bad position (altitude less than zero, GPS reports bad fix),
// then transmit all zeros.
#define FLAG_BAD_PACKET

// Lowers the transmit output power to defined value while awaiting GPS lock.
// Also lowers transmit interval.
// This can help with obtaining a faster lock, as it doesn't desense the receiver as much.
#define QUICK_LOCK

// Power level to use during quick lock mode.
#define QUICK_LOCK_POWER 30

#define QUICK_LOCK_INTERVAL 5000

// If defined, the tracker will transmit less frequently when on the ground to save power.
#define RECOVERY_MODE

// Interval to transmit in recovery mode, when on the ground.
#define RECOVERY_MODE_INTERVAL 120000

#define RECOVERY_MODE_FLIGHT_THRESHOLD 5000 // Altitude threshold to consider ourselves in flight, in meters.
#define RECOVERY_MODE_LAND_THRESHOLD 300 // Altitude threshold to consider ourselves on the ground, in meters.

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