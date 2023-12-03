//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tiny4FSK                                                                                             //
// The lightweight, small Horus Binary v2 4FSK tracker                                                  //
//                                                                                                      //
// Horus Binary modulation has been developed Mark Jessop and the Project Horus team                    //
// Made by Max Kendall W0MXX and the New England Weather Balloon Society (N.E.W.B.S.)                   //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <RadioLib.h>
#include "horus_l2.h"
#include <Wire.h>  //Needed for I2C to GNSS

#include <SparkFun_u-blox_GNSS_v3.h>  //http://librarymanager/All#SparkFun_u-blox_GNSS_v3

// RADIO SETUP

#define TX_FREQ 433.200
#define FSK4_BAUD 100
#define FSK4_SPACING 270  // NOTE: This results in a shift of 244 Hz due to the PLL Resolution of the SX127x

// SERIAL SETUP

#define Serial SerialUSB

// SX1278 has the following connections:
// NSS pin:   10
// DIO0 pin:  2
// RESET pin: 9
// DIO1 pin:  3
SX1278 radio = new Module(10, 2, 9, 3);

SFE_UBLOX_GNSS myGNSS;  // SFE_UBLOX_GNSS uses I2C. For Serial or SPI, see Example2 and Example3


// Horus Binary Structures & Variables

// Horus v2 Mode 1 (32-byte) Binary Packet
struct HorusBinaryPacketV2 {
  uint16_t PayloadID;
  uint16_t Counter;
  uint8_t Hours;
  uint8_t Minutes;
  uint8_t Seconds;
  float Latitude;
  float Longitude;
  uint16_t Altitude;
  uint8_t Speed;  // Speed in Knots (1-255 knots)
  uint8_t Sats;
  int8_t Temp;          // Twos Complement Temp value.
  uint8_t BattVoltage;  // 0 = 0.5v, 255 = 2.0V, linear steps in-between.
  // The following 9 bytes (up to the CRC) are user-customizable. The following just
  // provides an example of how they could be used.
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
double divisor = 10000000;


uint16_t crc_xmodem_update(uint16_t crc, uint8_t data) {
  int i;
  crc = crc ^ ((uint16_t)data << 8);
  for (i = 0; i < 8; i++) {
    if (crc & 0x8000)
      crc = (crc << 1) ^ 0x1021;
    else
      crc <<= 1;
  }
  return crc;
}

unsigned int crc16(unsigned char *string, unsigned int len) {
  unsigned int i;
  unsigned int crc;
  crc = 0xFFFF;  // Standard CCITT seed for CRC16.
  // Calculate the sum, ignore $ sign's
  for (i = 0; i < len; i++) {
    crc = crc_xmodem_update(crc, (uint8_t)string[i]);
  }
  return crc;
}

int build_horus_binary_packet_v2(char *buffer, int hours, int minutes, int seconds, float lat, float lon, int alt, int speed, double volts, int sats, int temp = 0) {
  // Generate a Horus Binary v2 packet, and populate it with data.
  // The assignments in this function should be replaced with real data

  struct HorusBinaryPacketV2 BinaryPacketV2;

  BinaryPacketV2.PayloadID = 380;
  BinaryPacketV2.Counter = packet_count;
  BinaryPacketV2.Hours = hours;
  BinaryPacketV2.Minutes = minutes;
  BinaryPacketV2.Seconds = seconds;
  BinaryPacketV2.Latitude = (lat);  // Corrected data type
  BinaryPacketV2.Longitude = (lon); // Corrected data type
  BinaryPacketV2.Altitude = alt;
  BinaryPacketV2.Speed = speed;
  BinaryPacketV2.BattVoltage = volts;
  BinaryPacketV2.Sats = sats;
  BinaryPacketV2.Temp = temp;
  BinaryPacketV2.dummy1 = 1;
  BinaryPacketV2.dummy2 = 2;
  BinaryPacketV2.dummy3 = 3;
  BinaryPacketV2.dummy4 = 4;
  BinaryPacketV2.dummy5 = 5;

  BinaryPacketV2.Checksum = (uint16_t)crc16((unsigned char *)&BinaryPacketV2, sizeof(BinaryPacketV2) - 2);

  memcpy(buffer, &BinaryPacketV2, sizeof(BinaryPacketV2));

  return sizeof(struct HorusBinaryPacketV2);
}



void setup() {

  Serial.begin(9600);
  delay(100);
  // initialize SX1278 with default settings
  Serial.println(("[SX1278] Initializing ... "));
  delay(100);


  // when using one of the non-LoRa modules for FSK4
  // (RF69, CC1101, Si4432 etc.), use the basic begin() method
  // int state = radio.begin();

  Serial.print(("[FSK4] Initializing ... "));

  // initialize FSK4 transmitter
  // NOTE: FSK4 frequency shift will be rounded
  //       to the nearest multiple of frequency step size.
  //       The exact value depends on the module:
  //         SX127x/RFM9x - 61 Hz
  //         RF69 - 61 Hz
  //         CC1101 - 397 Hz
  //         SX126x - 1 Hz
  //         nRF24 - 1000000 Hz
  //         Si443x/RFM2x - 156 Hz
  //         SX128x - 198 Hz
  Wire.begin();

  while (myGNSS.begin() == false)  //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Retrying..."));
    delay(1000);
  }

  myGNSS.setI2COutput(COM_TYPE_UBX);

  radio.beginFSK();
  fsk4_setup(&radio, TX_FREQ, FSK4_SPACING, FSK4_BAUD);
}

void loop() {
  int coded_len;
  int pkt_len;
  // Horus Binary V2
  Serial.println(F("Generating Horus Binary v2 Packet"));
  // Generate packet
  // Define variables to store previous location values
  float prevLatitude = 0.0;
  float prevLongitude = 0.0;
  int prevAltitudeMSL = 0.0;
  int prevHour = 0;
  int prevMinute = 0;
  int prevSecond = 0;
  double prevGroundSpeed = 0.0;
  

  // Inside your loop or wherever you are checking for GPS fix
  if (myGNSS.getPVT()) {
    // Update previous location values
    prevLatitude = myGNSS.getLatitude() / divisor;
    prevLongitude = myGNSS.getLongitude() / divisor;
    prevAltitudeMSL = myGNSS.getAltitudeMSL();
    prevHour = myGNSS.getHour();
    prevMinute = myGNSS.getMinute();
    prevSecond = myGNSS.getSecond();
    prevGroundSpeed = myGNSS.getGroundSpeed();
    Serial.print("Lat: ");
    Serial.print(prevLatitude);
    Serial.print(" ,Long: ");
    Serial.print(prevLongitude);
    Serial.print(" ,Alt: ");
    Serial.println(prevAltitudeMSL);

    // Build the packet with current GPS values
    pkt_len = build_horus_binary_packet_v2(rawbuffer, prevHour, prevMinute, prevSecond, prevLatitude, prevLongitude, prevAltitudeMSL, prevGroundSpeed, 5.00, 2);
  } else {
    // Use previous known location values if GPS fix is lost
    pkt_len = build_horus_binary_packet_v2(rawbuffer, prevHour, prevMinute, prevSecond, prevLatitude, prevLongitude, prevAltitudeMSL, prevGroundSpeed, 5.00, 2);
  }
  coded_len = horus_l2_encode_tx_packet((unsigned char *)codedbuffer, (unsigned char *)rawbuffer, pkt_len);
  // Transmit!
  Serial.println(F("Transmitting Horus Binary v2 Packet"));

  // send out idle condition for 1000 ms
  fsk4_idle(&radio);
  delay(1000);
  fsk4_preamble(&radio, 8);
  fsk4_write(&radio, codedbuffer, coded_len);

  Serial.println(F("done!"));

  delay(1000);

  packet_count++;
}
