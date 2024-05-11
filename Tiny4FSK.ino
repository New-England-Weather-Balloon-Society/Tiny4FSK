#include <Adafruit_BMP280.h>
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tiny4FSK                                                                                             //
// The lightweight, small Horus Binary v2 4FSK tracker                                                  //
//                                                                                                      //
// Horus Binary modulation has been developed by Mark Jessop and the Project Horus team                 //
// Made by Max Kendall W0MXX and the New England Weather Balloon Society (N.E.W.B.S.)                   //
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  MIT License

  Copyright (c) 2023 New England Weather Balloon Society

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

// ***************
// || Libraries ||
// ***************
#include <Wire.h>
#include <SparkFun_u-blox_GNSS_v3.h>
#include <ArduinoLowPower.h>
// #include <RadioLib.h>
#include <TinyBME280.h>
#include "horus_l2.h"
#include "config.h"
#include "crc_calc.h"
#include "voltage.h"
#include "i2c_scan.h"
#include "si4063.h"
#include <SPI.h>
#include "Adafruit_ZeroTimer.h"

// **********************
// || Native USB Setup ||
// **********************
#ifdef DEV_MODE
#define Serial SerialUSB
#endif

// *************************
// || Object Declarations ||
// *************************
// SX1278 radio module instance
// SX1278 radio = new Module(NSS_PIN, DIO0_PIN, RESET_PIN, DIO1_PIN);

// New UBLOX object
SFE_UBLOX_GNSS gps;

// Horus Binary Structures & Variables

// Horus v2 Structure of Packet
struct HorusBinaryPacketV2
{
  uint16_t PayloadID;
  uint16_t Counter;
  uint8_t Hours;
  uint8_t Minutes;
  uint8_t Seconds;
  float Latitude;
  float Longitude;
  uint16_t Altitude;
  uint8_t Speed; // Speed in Knots (1-255 knots)
  uint8_t Sats;
  int8_t Temp;
  uint8_t BattVoltage; // 0 = 0.5v, 255 = 2.0V, linear steps in-between.
  // The following 9 bytes (up to the CRC) are user-customizable.
  uint8_t dummy1;    // unsigned int
  float dummy2;      // Float
  uint8_t dummy3;    // battery voltage test
  uint8_t dummy4;    // divide by 10
  uint16_t dummy5;   // divide by 100
  uint16_t Checksum; // CRC16-CCITT Checksum.
} __attribute__((packed));

// Buffers and counters.
char rawbuffer[128];       // Buffer to temporarily store a raw binary packet.
char codedbuffer[128];     // Buffer to store an encoded binary packet
char debugbuffer[256];     // Buffer to store debug strings
uint16_t packet_count = 1; // Packet counter

#define NSEL 10
#define SDN 11

int count = 0;

float freq = 100.0;

volatile bool packetSent = 0;

// Adafruit_ZeroTimer zerotimer = Adafruit_ZeroTimer(3);

void setup()
{
  Serial.begin(9600);
  while(!Serial);

  // Initialize SX1278 radio with default settings
  Serial.println(("Initializing Radio..."));

  // Pinmode Declarations
  pinMode(ERROR_LED, OUTPUT);
  pinMode(SUCCESS_LED, OUTPUT);
  pinMode(NSEL, OUTPUT);
  pinMode(SDN, OUTPUT);

  // Initialize I2C for GPS
  Wire.begin();

  // Connect to u-blox GPS module
  while (gps.begin() == false)
  {
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
    showI2CAddresses();
  }
#ifdef STATUS_LED
  digitalWrite(ERROR_LED, LOW);
#endif

  // Configure GPS settings
  gps.setI2COutput(COM_TYPE_UBX);
  gps.factoryDefault();

  // ***********************
  // || GPS Configuration ||
  // ***********************
  gps.factoryDefault(); // Clear any saved configuration

  bool setValueSuccess = true;

  gps.enableGNSS(true, SFE_UBLOX_GNSS_ID_GPS);      // Enable GPS, disable everything else for lower power.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_SBAS);    // Disable SBAS.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_GALILEO); // Disable Galileo.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_BEIDOU);  // Disable BeiDou.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_IMES);    // Disable IMES.
  gps.enableGNSS(false, SFE_UBLOX_GNSS_ID_QZSS);    // Disable QZSS.
  gps.enableGNSS(true, SFE_UBLOX_GNSS_ID_GLONASS);  // Disable GLONASS.

  // setValueSuccess &= gps.setVal8(UBLOX_CFG_PM_OPERATEMODE, 2);   // Setting to PSMCT.
  setValueSuccess &= gps.setDynamicModel(DYN_MODEL_AIRBORNE1g); // Setting Airborne Mode.

  // Print GPS configuration status
  if (setValueSuccess == true)
  {
    gps.saveConfiguration();
#ifdef DEV_MODE
    Serial.println("GPS Config Success!");
#endif
#ifdef STATUS_LED
    digitalWrite(SUCCESS_LED, HIGH);
    delay(1000);
    digitalWrite(SUCCESS_LED, LOW);
#endif
  }
  else
  {
    while (1)
    {
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

  digitalWrite(EXTINT, LOW);

  // **********************
  // || Initialize Radio ||
  // **********************
  // radio.beginFSK();
  // fsk4_setup(&radio, FSK_FREQ, FSK_SPACING, FSK_BAUD);
  configureSi4063();
#ifdef DEV_MODE
  Serial.println(F("Radio Initialized!"));
#endif
#ifdef STATUS_LED
  digitalWrite(SUCCESS_LED, HIGH);
  delay(1000);
  digitalWrite(SUCCESS_LED, LOW);
#endif
  /*   BME280setI2Caddress(0x76);
    BME280setup(); */

  /*  uint16_t divider = 8;
    uint16_t compare = (48000000 / 8) / freq;
    tc_clock_prescaler prescaler = TC_CLOCK_PRESCALER_DIV8;

    zerotimer.enable(false);
    zerotimer.configure(prescaler,                   // prescaler
                        TC_COUNTER_SIZE_16BIT,       // bit width of timer/counter
                        TC_WAVE_GENERATION_MATCH_PWM // frequency or PWM mode
    );

    zerotimer.setCompare(0, compare);
    zerotimer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, packetHandler); */
}

void loop()
{
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

  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  si4063_enable_tx();
  SPI.endTransaction();

  fsk4_preamble(8);
  fsk4_write(codedbuffer, coded_len);

  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  //si4063_wait_for_cts();
  
  si4063_set_state(SI4063_STATE_READY);

  //si4063_wait_for_cts();
  SPI.endTransaction();

#ifdef DEV_MODE
  Serial.println(F("Transmission complete!"));
#endif
#ifdef STATUS_LED
  digitalWrite(SUCCESS_LED, HIGH);
  delay(1000);
  digitalWrite(SUCCESS_LED, LOW);
#endif

  // **********************
  // || Sleep Mode Time! ||
  // **********************
#ifndef DEV_MODE
  LowPower.deepSleep(1000);
#endif
#ifdef DEV_MODE
  delay(1000);
#endif
  packet_count++;
}

// Build the Horus v2 Packet. This is where the GPS positions and telemetry are organized to the struct.
int build_horus_binary_packet_v2(char *buffer)
{
  struct HorusBinaryPacketV2 BinaryPacketV2;

  // Wake up the BME280
  // BME280setup();

  // Fill with GPS readings.
  BinaryPacketV2.Hours = gps.getHour();
  BinaryPacketV2.Minutes = gps.getMinute();
  BinaryPacketV2.Seconds = gps.getSecond();
  BinaryPacketV2.Latitude = gps.getLatitude() / 10000000.00;
  BinaryPacketV2.Longitude = gps.getLongitude() / 10000000.00;
  BinaryPacketV2.Altitude = gps.getAltitudeMSL() / 100;
  BinaryPacketV2.Speed = gps.getGroundSpeed();
  BinaryPacketV2.Sats = gps.getSIV();
#ifdef STATUS_LED
  digitalWrite(SUCCESS_LED, HIGH);
  delay(1000);
  digitalWrite(SUCCESS_LED, LOW);
#endif

  // Non-GPS values
  BinaryPacketV2.PayloadID = HORUS_ID;
  BinaryPacketV2.Counter = packet_count;
  BinaryPacketV2.BattVoltage = readVoltage();
  BinaryPacketV2.Temp = 20; // BME280temperature() / 100.00;

  // User-Customizable Fields
  BinaryPacketV2.dummy1 = 1; // BME280pressure() / 100.00;
  BinaryPacketV2.dummy2 = 2; // BME280humidity() / 100.00;
  BinaryPacketV2.dummy3 = 3;
  BinaryPacketV2.dummy4 = 4;
  BinaryPacketV2.dummy5 = 5;

  // End the packet off with a CRC checksum.
  BinaryPacketV2.Checksum = (uint16_t)crc16((unsigned char *)&BinaryPacketV2, sizeof(BinaryPacketV2) - 2);

// Print some debug information if enabled
#ifdef DEV_MODE
  Serial.print("Latitude: ");
  Serial.print(BinaryPacketV2.Latitude, 7);
  Serial.print(", Longitude: ");
  Serial.print(BinaryPacketV2.Longitude, 7);
  Serial.print(", Sats: ");
  Serial.print(gps.getSIV());
  Serial.print(", Voltage: ");
  Serial.println(readVoltage());
/*   Serial.print(", Temperature: ");
  Serial.print(BME280temperature() / 100.00);
  Serial.print(", Pressure: ");
  Serial.print(BME280pressure() / 100.00);
  Serial.print(", Humidity: ");
  Serial.println(BME280humidity() / 100.00); */
#endif

  // Put the BME280 back to sleep
  // BME280sleep();

  // Copy the binary packet to the buffer
  memcpy(buffer, &BinaryPacketV2, sizeof(BinaryPacketV2));

  return sizeof(struct HorusBinaryPacketV2);
}

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
  rf_params.frequency_hz = 434000000;
  rf_params.power = 0x01;
  rf_params.type = SI4063_MODULATION_TYPE_CW;
  rf_params.offset = 0;
  rf_params.deviation_hz = 0x00;

  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  if (si4063_init(rf_params, si_params) != HAL_OK) {
    // Handle initialization error
    Serial.println("Initialization Error!");
    Serial.print("Part Number: 0x"); Serial.println(si4063_read_part_info(), HEX);
    while(1);
  }
  delay(100);
  Serial.print("Part Number: 0x"); Serial.println(si4063_read_part_info(), HEX);
  //si4063_enable_tx();
  SPI.endTransaction();
}

void PrintHex(char *data, uint8_t length, char *tmp)
{
  // Print char data as hex
  byte first;
  int j = 0;
  for (uint8_t i = 0; i < length; i++)
  {
    first = ((uint8_t)data[i] >> 4) | 48;
    if (first > 57)
      tmp[j] = first + (byte)39;
    else
      tmp[j] = first;
    j++;

    first = ((uint8_t)data[i] & 0x0F) | 48;
    if (first > 57)
      tmp[j] = first + (byte)39;
    else
      tmp[j] = first;
    j++;
  }
  tmp[length * 2] = 0;
}

size_t fsk4_writebyte(uint8_t b)
{
  // Send symbols MSB first.
  for (int k = 0; k < 4; k++)
  {
    // Extract 4FSK symbol (2 bits)
    uint8_t symbol = (b & 0xC0) >> 6;
    uint32_t start = micros();
    // Modulate
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
    si4063_set_frequency_offset(22 * symbol);
    SPI.endTransaction();

    while(micros() - start < 10000) {
      yield();
    }
    // Shift to next symbol.
    b = b << 2;
  }

  return (1);
}

void fsk4_write(char *buff, size_t len)
{
  for (size_t i = 0; i < len; i++)
  {
    fsk4_writebyte(buff[i]);
  }
Serial.println("433");
  //SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  //si4063_inhibit_tx();
  //SPI.endTransaction();
}
void fsk4_preamble(uint8_t len)
{
  int k;
  for (k = 0; k < len; k++)
  {
    fsk4_writebyte(0x1B);
  }
}

void fsk4_idle() {
  si4063_set_frequency_offset(0);
}
