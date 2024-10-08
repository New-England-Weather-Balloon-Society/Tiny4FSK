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
#include <SparkFun_u-blox_GNSS_v3.h>
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

// New UBLOX object
SFE_UBLOX_GNSS gps;

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
  // The following bytes (up until the CRC) are user-customizable. These can be changed by using a custom field list (see horusdemodlib)
  int16_t AscentRate; // Divide by 100
  int16_t ExtTemp; // Divide by 10
  uint8_t Humidity; // No post-processing
  uint16_t ExtPress; // Divide by 10
  uint8_t dummy1;
  uint8_t dummy2;
  uint16_t Checksum;
} __attribute__((packed));

// Buffers and counters.
char rawbuffer[128];        // Buffer to temporarily store a raw binary packet.
char codedbuffer[128];      // Buffer to store an encoded binary packet
char debugbuffer[256];      // Buffer to store debug strings
uint16_t packet_count = 1;  // Packet counter
int call_count = 0;         // Counter to sense when to send callsign
uint16_t pastAltitude = 0;  // For ascent rate calculation

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

  // Initialize I2C for GPS
  Wire.begin();

  // Connect to u-blox GPS module
  while (gps.begin() == false) {
#ifdef DEV_MODE
    Serial.println(F("u-blox GNSS not detected at default I2C address. Printing I2C scan. Restarting..."));
#endif
    delay(1000);
#ifdef STATUS_LED
    digitalWrite(ERROR_LED, HIGH);
    delay(500);
    digitalWrite(ERROR_LED, LOW);
    delay(500);
#endif
#ifdef DEV_MODE
    showI2CAddresses();
#endif
  }
#ifdef STATUS_LED
  digitalWrite(ERROR_LED, LOW);
#endif

  // ***********************
  // || GPS Configuration ||
  // ***********************
  gps.setI2COutput(COM_TYPE_UBX);
  gps.factoryDefault();

  bool setValueSuccess = true;

  gps.enableGNSS(true, SFE_UBLOX_GNSS_ID_GPS);       // Enable GPS, disable everything else for lower power.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_SBAS);     // Disable SBAS.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_GALILEO);  // Disable Galileo.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_BEIDOU);   // Disable BeiDou.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_IMES);     // Disable IMES.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_QZSS);     // Disable QZSS.
  gps.enableGNSS(true, SFE_UBLOX_GNSS_ID_GLONASS);   // Disable GLONASS.

  // setValueSuccess &= gps.setVal8(UBLOX_CFG_PM_OPERATEMODE, 2);   // Setting to PSMCT.
  setValueSuccess &= gps.setDynamicModel(DYN_MODEL_AIRBORNE1g);  // Setting Airborne Mode.

  // Print GPS configuration status
  if (setValueSuccess == true) {
    gps.saveConfiguration();
#ifdef DEV_MODE
    Serial.println("GPS Config Success!");
#endif
#ifdef STATUS_LED
    digitalWrite(SUCCESS_LED, HIGH);
    delay(1000);
    digitalWrite(SUCCESS_LED, LOW);
#endif
  } else {
    while (1) {
#ifdef DEV_MODE
      Serial.println("GPS Config Failed!");
#endif
#ifdef STATUS_LED
      digitalWrite(ERROR_LED, HIGH);
      delay(500);
      digitalWrite(ERROR_LED, LOW);
      delay(500);
#endif
    }
  }

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
  BME280setup();

  // ******************************
  // || Send Morse Code Callsign ||
  // ******************************
  sendCallsign();
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
  
// Fill with GPS readings, with a GPS sanity check
#ifdef FLAG_BAD_PACKET
  if (gps.getAltitudeMSL() > 0 && gps.getGnssFixOk()) {
    BinaryPacketV2.Hours = gps.getHour();
    BinaryPacketV2.Minutes = gps.getMinute();
    BinaryPacketV2.Seconds = gps.getSecond();
    BinaryPacketV2.Latitude = gps.getLatitude() / 10000000.00;
    BinaryPacketV2.Longitude = gps.getLongitude() / 10000000.00;
    BinaryPacketV2.Altitude = gps.getAltitudeMSL() / 1000.00;
    BinaryPacketV2.Speed = gps.getGroundSpeed();
    BinaryPacketV2.Sats = gps.getSIV();
    BinaryPacketV2.AscentRate = (pastAltitude / BinaryPacketV2.Altitude) / PACKET_INTERVAL;
  } else {
    BinaryPacketV2.Hours = 0;
    BinaryPacketV2.Minutes = 0;
    BinaryPacketV2.Seconds = 0;
    BinaryPacketV2.Latitude = 0;
    BinaryPacketV2.Longitude = 0;
    BinaryPacketV2.Altitude = 0;
    BinaryPacketV2.Speed = 0;
    BinaryPacketV2.Sats = gps.getSIV();
  }
#else

  // Or, if you prefer no sanity check, force GPS positions into struct
  BinaryPacketV2.Hours = gps.getHour();
  BinaryPacketV2.Minutes = gps.getMinute();
  BinaryPacketV2.Seconds = gps.getSecond();
  BinaryPacketV2.Latitude = gps.getLatitude() / 10000000.00;
  BinaryPacketV2.Longitude = gps.getLongitude() / 10000000.00;
  BinaryPacketV2.Altitude = gps.getAltitudeMSL() / 1000.00;
  BinaryPacketV2.Speed = gps.getGroundSpeed() * 10;
  BinaryPacketV2.Sats = gps.getSIV();
#endif
#ifdef STATUS_LED
  digitalWrite(SUCCESS_LED, HIGH);
  delay(500);
  digitalWrite(SUCCESS_LED, LOW);
#endif

  // Non-GPS values
  BinaryPacketV2.PayloadID = HORUS_ID;
  BinaryPacketV2.Counter = packet_count;
  BinaryPacketV2.BattVoltage = (int)mapf((double)readVoltage(), 0.00, 5.00, 0, 255);
  BinaryPacketV2.Temp = BME280temperature() / 100.00;

  // User-Customizable Fields
  BinaryPacketV2.AscentRate = 0;
  BinaryPacketV2.ExtTemp = (int16_t)(BME280temperature() / 10);
  BinaryPacketV2.Humidity = (int8_t)(BME280humidity() / 100);
  BinaryPacketV2.ExtPress = (int16_t)(BME280pressure() / 10);

  // End the packet off with a CRC checksum.
  BinaryPacketV2.Checksum = (uint16_t)crc16((unsigned char *)&BinaryPacketV2, sizeof(BinaryPacketV2) - 2);

  // Dump the sensor values to Serial Monitor
#ifdef DEV_MODE
  Serial.print("Frame Count: ");
  Serial.print(packet_count);
  Serial.print(", Latitude: ");
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
  Serial.print(BinaryPacketV2.Temp);
  Serial.print(", External Temperature: ");
  Serial.print(BinaryPacketV2.ExtTemp);
  Serial.print(", Pressure: ");
  Serial.print(BinaryPacketV2.ExtPress);
  Serial.print(", Humidity: ");
  Serial.println(BinaryPacketV2.Humidity);
#endif

  pastAltitude = BinaryPacketV2.Altitude / 1000;

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
  rf_params.power = OUTPUT_POWER;
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