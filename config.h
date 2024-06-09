// TINY4FSK CONFIG FILE
// Please read comments for each section, even though some settings are self-explanatory.
// Feel free to add an issue or PR on GitHub for any questions or bugs.

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
#define HORUS_ID 256 // Replace with the assigned Horus v2 ID

// FSK Center Frequency in MHz. Ensure SDR is tuned to this frequency.
#define FSK_FREQ 433.200

// Baud Rate of FSK Packet. No need to change.
#define FSK_BAUD 100

// Spacing of FSK peaks. Adjust in the decoding program (e.g., Horus GUI, HorusDemodLib).
#define FSK_SPACING 270

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

// You most likely do not need to touch these, if you are using the PCB.
// Mostly for individual development or shields.

// GPS External interrupt Pin. In junction with I2C pins.
#define EXTINT 8

// Status LED Pins
#define ERROR_LED 5
#define SUCCESS_LED 4
