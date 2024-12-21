//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tiny4FSK                                                                                             //
// The lightweight, small Horus Binary v2 4FSK tracker                                                  //
//                                                                                                      //
// Horus Binary modulation has been developed by Mark Jessop and the Project Horus team                 //
// Made by Max Kendall W0MXX and the New England Weather Balloon Society (N.E.W.B.S.)                   //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// ***************
// || Libraries ||
// ***************
#include <Wire.h>
#include <SPI.h>
#include <ArduinoLowPower.h>
#include <TinyGPSPlus.h>
#include <TinyBME280.h>
#include "horus_l2.h"
#include "config.h"
#include "crc_calc.h"
#include "voltage.h"
#include "i2c_scan.h"
#include "si4063.h"
#include "4fsk_mod.h"
#include "morse.h"

// **********************
// || Native USB Setup ||
// **********************
#ifdef DEV_MODE
#define Serial SerialUSB
#endif

// ***************************
// || Variable Declarations ||
// ***************************

// New GPS object
TinyGPSPlus gps;

// Horus Binary Structures & Variables

// Horus v2 Structure of Packet
// https://github.com/projecthorus/horusdemodlib/wiki/4-Packet-Format-Details#packet-formats
struct HorusBinaryPacketV2 {
  uint16_t PayloadID;
  uint16_t Counter;
  uint8_t Hours;
  uint8_t Minutes;
  uint8_t Seconds;
  float Latitude;
  float Longitude;
  uint16_t Altitude;
  uint8_t Speed;
  uint8_t Sats;
  int8_t Temp;
  uint8_t BattVoltage;
  // The following bytes (up until the CRC) are user-customizable.
  uint8_t dummy1;
  float dummy2;
  uint8_t dummy3;
  uint8_t dummy4;
  uint16_t dummy5;
  uint16_t Checksum;
} __attribute__((packed));

// Buffers and counters.
char rawbuffer[128];        // Buffer to temporarily store a raw binary packet.
char codedbuffer[128];      // Buffer to store an encoded binary packet
char debugbuffer[256];      // Buffer to store debug strings
uint16_t packet_count = 1;  // Packet counter
int call_count = 0;         // Counter to sense when to send callsign

// Make sure interval is at the legal limit!
#if CALLSIGN_INTERVAL > 600000
#error "Please set the CALLSIGN_INTERVAL to less than or equal to 10 minutes to keep this legal!
#endif

void setup() {
  // ****************************
  // || Runtime Initialization ||
  // ****************************
  // Begin the Serial Monitor
#ifdef DEV_MODE
  Serial.begin(9600);
#endif

  // Initialize Si4063 radio with default settings
  Serial.println(("Initializing Radio..."));

  // Pinmode Declarations
  pinMode(ERROR_LED, OUTPUT);
  pinMode(SUCCESS_LED, OUTPUT);
  pinMode(NSEL, OUTPUT);
  pinMode(SDN, OUTPUT);

  // Initialize SPI for Si4063
  SPI.begin();

  // Initialize Serial1 for GPS
  Serial1.begin(9600);

  // Connect to GPS module
  while (!gps.location.isValid() && millis() < 5000) {
    while (Serial1.available() > 0) {
      gps.encode(Serial1.read());
    }
  }

  Serial.println("GPS detected!");

  // ***********************
  // || GPS Configuration ||
  // ***********************

  // Set to Airborne Mode (<1g)
  Serial1.write("$PCAS11,5*18\r\n");

#ifdef STATUS_LED
  digitalWrite(SUCCESS_LED, HIGH);
  delay(1000);
  digitalWrite(SUCCESS_LED, LOW);
#endif


  // **********************
  // || Initialize Radio ||
  // **********************
  configureSi4063();
#ifdef DEV_MODE
  Serial.println(F("Radio Initialized!"));
#endif
#ifdef STATUS_LED
  digitalWrite(SUCCESS_LED, HIGH);
  delay(1000);
  digitalWrite(SUCCESS_LED, LOW);
#endif

  // ***********************
  // || Initialize BME280 ||
  // ***********************
  Wire.begin();
  BME280setI2Caddress(0x76);
  BME280setup();
}

void loop() {
  // ***************************
  // || Callsign Transmission ||
  // ***************************

  // Check if it's the right time to send the callsign
  if (call_count * PACKET_INTERVAL >= CALLSIGN_INTERVAL) {
    // Send the callsign, and reset the counter
    sendCallsign();
    call_count = 0;
  }

  // *********************    
  // || Local Variables ||
  // *********************
  int coded_len;
  int pkt_len;

  // ***************************
  // || Generate Horus Packet ||
  // ***************************
#ifdef DEV_MODE
  Serial.println(F("Generating Horus Binary v2 Packet"));
#endif
  pkt_len = build_horus_binary_packet_v2(rawbuffer);
  coded_len = horus_l2_encode_tx_packet((unsigned char *)codedbuffer, (unsigned char *)rawbuffer, pkt_len);

  // *******************
  // || Transmit Time ||
  // *******************
#ifdef DEV_MODE
  Serial.println(F("Transmitting Horus Binary v2 Packet"));
#endif

  // Start sending out a continuous signal
  si4063_enable_tx();

  // Take the buffer, convert to symbols 0-3, and send them by setting the frequency
  fsk4_preamble(8);
  fsk4_write(codedbuffer, coded_len);

  // End the transmission
  si4063_inhibit_tx();

#ifdef DEV_MODE
  Serial.println(F("Transmission complete!"));
#endif
#ifdef STATUS_LED
  digitalWrite(SUCCESS_LED, HIGH);
  delay(500);
  digitalWrite(SUCCESS_LED, LOW);
#endif

  // Increment packet counter
  packet_count++;
  call_count++;

  // **********************
  // || Sleep Mode Time! ||
  // **********************
#ifndef DEV_MODE
  LowPower.deepSleep(PACKET_INTERVAL);
#endif
#ifdef DEV_MODE
  delay(PACKET_INTERVAL);
#endif
}


// **********************
// || Custom Functions ||
// **********************

// Build the Horus v2 Packet. This is where the GPS positions and telemetry are organized to the struct.
int build_horus_binary_packet_v2(char *buffer) {
  struct HorusBinaryPacketV2 BinaryPacketV2;
   while (Serial1.available() > 0) {
      gps.encode(Serial1.read());
    }
// Fill with GPS readings, with a GPS sanity check
#ifdef FLAG_BAD_PACKET
  if (gps.altitude.meters() > 0 && gps.location.isValid()) {
    BinaryPacketV2.Hours = gps.time.hour();
    BinaryPacketV2.Minutes = gps.time.minute();
    BinaryPacketV2.Seconds = gps.time.second();
    BinaryPacketV2.Latitude = gps.location.lat();
    BinaryPacketV2.Longitude = gps.location.lng();
    BinaryPacketV2.Altitude = gps.altitude.meters();
    BinaryPacketV2.Speed = gps.speed.kmph();
    BinaryPacketV2.Sats = gps.satellites.value();
  } else {
    BinaryPacketV2.Hours = 0;
    BinaryPacketV2.Minutes = 0;
    BinaryPacketV2.Seconds = 0;
    BinaryPacketV2.Latitude = 0;
    BinaryPacketV2.Longitude = 0;
    BinaryPacketV2.Altitude = 0;
    BinaryPacketV2.Speed = 0;
    BinaryPacketV2.Sats = gps.satellites.value();
  }
#else
  // Or, if you prefer no sanity check, force GPS positions into struct
  BinaryPacketV2.Hours = gps.time.hour();
  BinaryPacketV2.Minutes = gps.time.minute();
  BinaryPacketV2.Seconds = gps.time.second();
  BinaryPacketV2.Latitude = gps.location.lat();
  BinaryPacketV2.Longitude = gps.location.lng();
  BinaryPacketV2.Altitude = gps.altitude.meters();
  BinaryPacketV2.Speed = gps.speed.kmph();
  BinaryPacketV2.Sats = gps.satellites.value();
#endif
#ifdef STATUS_LED
  digitalWrite(SUCCESS_LED, HIGH);
  delay(500);
  digitalWrite(SUCCESS_LED, LOW);
#endif

  // Non-GPS values
  BinaryPacketV2.PayloadID = HORUS_ID;
  BinaryPacketV2.Counter = packet_count;
  BinaryPacketV2.BattVoltage = (int)mapf((double)readVoltage(), 0.00, 3.30, 0, 255);
  BinaryPacketV2.Temp = BME280temperature() / 100.00;

  // User-Customizable Fields
  BinaryPacketV2.dummy1 = BME280pressure() / 100.00;
  BinaryPacketV2.dummy2 = BME280humidity() / 100.00;
  BinaryPacketV2.dummy3 = 0;
  BinaryPacketV2.dummy4 = 0;
  BinaryPacketV2.dummy5 = 0;

  // End the packet off with a CRC checksum.
  BinaryPacketV2.Checksum = (uint16_t)crc16((unsigned char *)&BinaryPacketV2, sizeof(BinaryPacketV2) - 2);

  // Dump the sensor values to Serial Monitor
#ifdef DEV_MODE
  Serial.print("Latitude: ");
  Serial.print(BinaryPacketV2.Latitude, 7);
  Serial.print(", Longitude: ");
  Serial.print(BinaryPacketV2.Longitude, 7);
  Serial.print(", Altitude MSL: ");
  Serial.print(BinaryPacketV2.Altitude);
  Serial.print(", Sats: ");
  Serial.print(BinaryPacketV2.Sats);
  Serial.print(", Voltage: ");
  Serial.print(readVoltage());
  Serial.print(", Voltage (Scaled): ");
  Serial.print(BinaryPacketV2.BattVoltage);
  Serial.print(", Temperature: ");
  Serial.print(BME280temperature() / 100.00);
  Serial.print(", Pressure: ");
  Serial.print(BME280pressure() / 100.00);
  Serial.print(", Humidity: ");
  Serial.println(BME280humidity() / 100.00);
#endif

  // Copy the binary packet to the buffer
  memcpy(buffer, &BinaryPacketV2, sizeof(BinaryPacketV2));

  return sizeof(struct HorusBinaryPacketV2);
}

// Configure the Si4063 to user values
void configureSi4063() {
  chip_parameters si_params;
  si_params.gpio0 = 0x00;
  si_params.gpio1 = 0x00;
  si_params.gpio2 = 0x00;
  si_params.gpio3 = 0x00;
  si_params.drive_strength = 0x00;
  si_params.clock = 26000000UL;

  radio_parameters rf_params;
  rf_params.frequency_hz = FSK_FREQ * 1000000;
  rf_params.power = 0x30;
  rf_params.type = SI4063_MODULATION_TYPE_CW;
  rf_params.offset = 0;
  rf_params.deviation_hz = 0x00;

  // Handle initialization error
  if (si4063_init(rf_params, si_params) != HAL_OK) {
#ifdef DEV_MODE
    Serial.println("Initialization Error!");
#endif
    while (1)
      ;
  }

  // Disable TX if in reset mode
  si4063_inhibit_tx();
}

// Custom map function that supports floating-point mapping
double mapf(double x, double in_min, double in_max, double out_min, double out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Send out the Morse Code callsign
void sendCallsign() {
#ifdef DEV_MODE
  Serial.println("Sending Morse Code Callsign!");
#endif
  si4063_set_frequency_offset(0);
  sendMorseString(CALLSIGN);
}