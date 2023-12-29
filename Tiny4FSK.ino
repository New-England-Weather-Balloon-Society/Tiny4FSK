//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tiny4FSK                                                                                             //
// The lightweight, small Horus Binary v2 4FSK tracker                                                  //
//                                                                                                      //
// Horus Binary modulation has been developed by Mark Jessop and the Project Horus team                 //
// Made by Max Kendall W0MXX and the New England Weather Balloon Society (N.E.W.B.S.)                   //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <RadioLib.h>
#include "horus_l2.h"
#include <Wire.h>
#include <SparkFun_u-blox_GNSS_v3.h>
#include "config.h"
#include "crc_calc.h"

// SERIAL SETUP
// Needed for SAMD Native USB.
#ifdef DEBUG_FEEDBACK
#define Serial SerialUSB
#endif

// SX1278 connections
const int NSS_PIN = 10;
const int DIO0_PIN = 2;
const int RESET_PIN = 9;
const int DIO1_PIN = 3;

// SX1278 radio module instance
SX1278 radio = new Module(NSS_PIN, DIO0_PIN, RESET_PIN, DIO1_PIN);

// New UBLOX object
SFE_UBLOX_GNSS gps;

// External interrupt pin
#define EXTINT 8

// Horus Binary Structures & Variables

// Horus v2 Structure of Packet
struct HorusBinaryPacketV2 {
  uint16_t PayloadID;
  uint16_t Counter;
  uint8_t Hours;
  uint8_t Minutes;
  uint8_t Seconds;
  float Latitude;
  float Longitude;
  uint16_t Altitude;
  uint8_t Speed;      // Speed in Knots (1-255 knots)
  uint8_t Sats;
  int8_t Temp;        // Twos Complement Temp value.
  uint8_t BattVoltage;  // 0 = 0.5v, 255 = 2.0V, linear steps in-between.
  // The following 9 bytes (up to the CRC) are user-customizable.
  uint8_t dummy1;     // unsigned int
  float dummy2;       // Float
  uint8_t dummy3;     // battery voltage test
  uint8_t dummy4;     // divide by 10
  uint16_t dummy5;    // divide by 100
  uint16_t Checksum;  // CRC16-CCITT Checksum.
} __attribute__((packed));

// Buffers and counters.
char rawbuffer[128];        // Buffer to temporarily store a raw binary packet.
char codedbuffer[128];      // Buffer to store an encoded binary packet
char debugbuffer[256];      // Buffer to store debug strings
uint16_t packet_count = 1;  // Packet counter

// Build the Horus v2 Packet. This is where the GPS positions are organized to the struct.
int build_horus_binary_packet_v2(char *buffer) {
  struct HorusBinaryPacketV2 BinaryPacketV2;

  // Wake up GPS by pulsing to the EXTINT (External Interrupt) pin. 250 ms is probably more the needed,
  // but nice to be safe.
  digitalWrite(EXTINT, HIGH);
  delay(250);
  digitalWrite(EXTINT, LOW);

  // Payload ID (16 bits)
  BinaryPacketV2.PayloadID = 256;

  // Counter (16 bits)
  BinaryPacketV2.Counter = packet_count;

  // Time Fields (Hours, Minutes, Seconds)
  BinaryPacketV2.Hours = gps.getHour();
  BinaryPacketV2.Minutes = gps.getMinute();
  BinaryPacketV2.Seconds = gps.getSecond();

  // GPS Coordinates (Latitude, Longitude)
  BinaryPacketV2.Latitude = gps.getLatitude() / 10000000.00;
  BinaryPacketV2.Longitude = gps.getLongitude() / 10000000.00;

  // Altitude (16 bits)
  BinaryPacketV2.Altitude = 70;

  // Speed (8 bits) - Speed in Knots (1-255 knots)
  BinaryPacketV2.Speed = gps.getGroundSpeed();

  // Battery Voltage (8 bits) - 0 = 0.5v, 255 = 2.0V, linear steps in-between
  BinaryPacketV2.BattVoltage = 5.00;  // Voltage Divider

  // Satellites (8 bits)
  BinaryPacketV2.Sats = gps.getSIV();

  // Temperature (8 bits) - Twos Complement Temp value
  BinaryPacketV2.Temp = 20;  // BME280

  // User-Customizable Fields (dummy1, dummy2, dummy3, dummy4, dummy5)
  BinaryPacketV2.dummy1 = 1;
  BinaryPacketV2.dummy2 = 2;
  BinaryPacketV2.dummy3 = 3;
  BinaryPacketV2.dummy4 = 4;
  BinaryPacketV2.dummy5 = 5;

  // Print some debug information
  Serial.print("Latitude: ");
  Serial.print(BinaryPacketV2.Latitude, 7);
  Serial.print(", Longitude: ");
  Serial.print(BinaryPacketV2.Longitude, 7);
  Serial.print(", Sats: ");
  Serial.println(gps.getSIV());

  // Calculate CRC checksum (16 bits)
  BinaryPacketV2.Checksum = (uint16_t)crc16((unsigned char *)&BinaryPacketV2, sizeof(BinaryPacketV2) - 2);

  // Copy the binary packet to the buffer
  memcpy(buffer, &BinaryPacketV2, sizeof(BinaryPacketV2));

  // Put GPS back to sleep.
  gps.powerOffWithInterrupt(10000, VAL_RXM_PMREQ_WAKEUPSOURCE_EXTINT0, true);

  return sizeof(struct HorusBinaryPacketV2);
}


void setup() {
  Serial.begin(9600);

  // Initialize SX1278 radio with default settings
  Serial.println(("Initializing Radio..."));

  // Initialize FSK4 transmitter
  Wire.begin();

  // Connect to u-blox GPS module
  while (gps.begin() == false) {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Retrying..."));
    delay(1000);
  }

  // Configure GPS settings
  gps.setI2COutput(COM_TYPE_UBX);
  gps.factoryDefault();

  // Configure GPS power-saving settings
  gps.factoryDefault();  // Clear any saved configuration

  bool setValueSuccess = true;

  gps.enableGNSS(true, SFE_UBLOX_GNSS_ID_GPS);       // Enable GPS, disable everything else for lower power and for PSMOO to work.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_SBAS);     // Disable SBAS.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_GALILEO);  // Disable Galileo.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_BEIDOU);   // Disable BeiDou.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_IMES);     // Disable IMES.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_QZSS);     // Disable QZSS.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_GLONASS);  // Disable GLONASS.

  setValueSuccess &= gps.setVal8(UBLOX_CFG_PM_OPERATEMODE, 2);        // Setting to PSMCT.

  // Print GPS configuration status
  if (setValueSuccess == true) {
    gps.saveConfiguration();
    Serial.println("GPS Config Success!");
  } else {
    Serial.println("GPS Config Failed!");
  }

  digitalWrite(EXTINT, LOW);

  // Initialize radio for FSK
  radio.beginFSK();
  fsk4_setup(&radio, FSK_FREQ, FSK_SPACING, FSK_BAUD);
  Serial.println(("Radio Initialized!"));
}

void loop() {
  int coded_len;
  int pkt_len;

  // Generate and transmit Horus Binary V2 packet
  Serial.println(F("Generating Horus Binary v2 Packet"));
  pkt_len = build_horus_binary_packet_v2(rawbuffer);
  coded_len = horus_l2_encode_tx_packet((unsigned char *)codedbuffer, (unsigned char *)rawbuffer, pkt_len);

  // Transmit!
  Serial.println(F("Transmitting Horus Binary v2 Packet"));
  fsk4_idle(&radio);  // send out idle condition for 1000 ms
  delay(1000);
  fsk4_preamble(&radio, 8);
  fsk4_write(&radio, codedbuffer, coded_len);

  Serial.println(F("Transmission complete!"));

  delay(1000);

  packet_count++;
}
