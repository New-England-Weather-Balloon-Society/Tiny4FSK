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

// PLEASE READ ALL COMMENTS, even though some settings may be self-explanatory.
// Feel free to add an issue or PR on GitHub for any questions or bugs.

#pragma once

// *****************************
// || Essential Configuration ||
// *****************************

// ---------------- CRITICAL ----------------
// Amateur radio license is required! Give callsign here.
#define CALLSIGN "N0CALL"

// FSK Center Frequency in MHz. Ensure SDR is tuned to this frequency.
#define FREQUENCY 432.634
// ------------------------------------------

// -------------- BATTERY LIFE --------------
// Delay between each packet, in milliseconds. Refer the the Battery Life Expectations section of the README for more information.
// For 90% of use cases, try to keep this over 10 seconds in favor of increasing battery life. The longer the interval, the longer the battery will last.
#define PACKET_INTERVAL 10000

// Enable status mode LEDs for information on GPS initialization and issues.
// Disabling will save some power.
#define STATUS_LED

// Keeping this setting enabled allows Serial prints to be sent to the Serial Monitor.
// This is useful for debugging, but will increase power consumption and reduce battery life.
// Disabling will allow the tracker to enter deep-sleep mode between transmissions, which will greatly increase battery life.
// If you're plugged into USB without DEV_MODE, you'll see it disconnect between transmissions. Check the README!!
#define DEV_MODE

// Sets the output power level from 0dBm - ~19dBm (0-127).
// Honestly, this really doesn't do much for battery life. Check the graph in the README.
#define OUTPUT_POWER 127
// ------------------------------------------

// ------------- LAUNCH/LANDING -------------
// Lowers the transmit output power to defined value while awaiting GPS lock. Also lowers transmit interval.
// This can help with obtaining a faster lock, as it doesn't desense the receiver as much.
#define QUICK_LOCK

// Power level to use during quick lock mode (0-127).
#define QUICK_LOCK_POWER 30

// Interval to transmit in quick lock mode, when waiting for a GPS fix, in milliseconds.
// It'd make the most sense to make this longer than PACKET_INTERVAL.
#define QUICK_LOCK_INTERVAL 5000


// If defined, the tracker will transmit less frequently when on the ground to save power.
#define RECOVERY_MODE

// Interval to transmit in recovery mode, when on the ground.
#define RECOVERY_MODE_INTERVAL 120000

#define RECOVERY_MODE_FLIGHT_THRESHOLD 5000 // Altitude threshold to consider ourselves in flight, in meters.
#define RECOVERY_MODE_LAND_THRESHOLD 300 // Altitude threshold to consider ourselves on the ground, in meters.
// ------------------------------------------







// ****************************
// || Advanced Configuration ||
// ****************************

// These settings modify the internal behavior of the board. You should not need to modify these unless you are doing custom development or have a specific use case.
// As always, please refer to the README.

// ********************
// | Tracker Settings |
// ********************

// Transmits callsign in CW to stay legal. Keep at 20 or below.
#define CALLSIGN_WPM 20
// Interval to send the CW in milliseconds.
#define CALLSIGN_INTERVAL 600000

// Baud Rate of FSK Packet.
#define FSK_BAUD 100
// Spacing of FSK peaks. Adjust in the decoding program (e.g., Horus GUI, HorusDemodLib).
#define FSK_SPACING 270

// If the GPS position seems to be a bad position (altitude less than zero, GPS reports bad fix),
// then transmit all zeros.
#define FLAG_BAD_PACKET

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