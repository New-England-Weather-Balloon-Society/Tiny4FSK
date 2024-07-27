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
#define HORUS_ID 554

// Amateur radio license required! Give callsign here.
#define CALLSIGN "KC1SFR"

// Transmits callsign in CW to stay legal. Keep at 20 or below.
#define CALLSIGN_WPM 20

// Interval to send the CW in milliseconds.
#define CALLSIGN_INTERVAL 600000

// FSK Center Frequency in MHz. Ensure SDR is tuned to this frequency.
#define FSK_FREQ 432.608

// Baud Rate of FSK Packet. No need to change.
#define FSK_BAUD 100

// Spacing of FSK peaks. Adjust in the decoding program (e.g., Horus GUI, HorusDemodLib).
#define FSK_SPACING 270

// Delay between each packet, in milliseconds.
#define PACKET_DELAY 10000

// If the GPS position seems to be a bad position (alititude less than zero, GPS reports bad fix),
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

// EXPERIMENTAL - optimise for EXTREMELY low power draw
//#define ULTRA_LOW_POWER

// *********************
// || Pin Definitions ||
// *********************

// You most likely do not need to touch these, if you are using the PCB.
// Mostly for individual development or shields.

// Si4063 Pins. These are in junction with normal SPI pins (MISO, MOSI, SCK).
#define NSEL_PIN 10
#define SDN_PIN 11

// GPS External interrupt Pin. In junction with I2C pins.
#define EXTINT_GPS 8

// Status LED Pins
#define ERROR_LED 5
#define SUCCESS_LED 4