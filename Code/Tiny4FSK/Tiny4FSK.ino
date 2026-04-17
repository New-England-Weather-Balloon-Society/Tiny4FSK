/*
Tiny4FSK.ino, part of Tiny4FSK, for a high-altitude tracker.
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
#include <Scheduler.h>
#include <SD.h>
#include "horus_l2.h"
#include "config.h"
#include "crc_calc.h"
#include "voltage.h"
#include "si4063.h"
#include "4fsk_mod.h"
#include "morse.h"
#include "utils.h"
#include "shield.h"
#include "horus_binary_v3.h"

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

// Horus Binary V3 Packet
telemetry_message TelemetryStruct;

// Buffers and counters.
char codedbuffer[HORUS_CODED_BUFFER_SIZE]; // Buffer to store an encoded binary packet
char debugbuffer[256];     // Buffer to store debug strings
uint16_t packet_count = 1; // Packet counter
int call_count = 0;        // Counter to sense when to send callsign

// Make sure interval is at the legal limit!
#if CALLSIGN_INTERVAL > 600000
#error "Please set the CALLSIGN_INTERVAL to less than or equal to 10 minutes to keep this legal!"
#endif

void setup()
{
  // ****************************
  // || Runtime Initialization ||
  // ****************************

  // Begin the Serial Monitor
#ifdef DEV_MODE
  Serial.begin(9600);
  // while (!Serial);
  Serial.println("Welcome to Tiny4FSK! Beginning initialization process.");
#endif

  // Pinmode Declarations
  pinMode(ERROR_LED, OUTPUT);
  pinMode(SUCCESS_LED, OUTPUT);
  pinMode(NSEL, OUTPUT);
  pinMode(SDN, OUTPUT);

  // ************************
  // || GPS Initialization ||
  // ************************

#ifdef DEV_MODE
  Serial.println("SD Card Initialized! Initializing GPS module...");
#endif

  // Initialize Serial1 for GPS
  Serial1.begin(9600);

  // Connect to GPS module
  int startTimer = millis();
  while (!gps.location.isValid() && millis() - startTimer < 1000)
  {
    while (Serial1.available() > 0)
    {
      gps.encode(Serial1.read());
    }
  }

#ifdef DEV_MODE
  Serial.println("GPS detected! Setting Airborne mode (<1g) configuration...");
#endif

  // Set to Airborne Mode (<1g) using CASIC11 command
  Serial1.write("$PCAS11,5*18\r\n");

#ifdef STATUS_LED
  digitalWrite(SUCCESS_LED, HIGH);
  delay(1000);
  digitalWrite(SUCCESS_LED, LOW);
#endif

  // **********************
  // || Initialize Radio ||
  // **********************

  // Initialize Si4063 radio with default settings
#ifdef DEV_MODE
  Serial.println("GPS initialized!\nInitializing radio...");
#endif

  // Initialize SPI for Si4063
  SPI.begin();
  configureSi4063();

#ifdef DEV_MODE
  Serial.println("Radio Initialized!");
#endif

  // ***********************
  // || Initialize BME280 ||
  // ***********************

#ifdef DEV_MODE
  Serial.println("Initializing BME280...");
#endif

  Wire.begin();
  BME280setI2Caddress(0x76);
  BME280setup();

#ifdef DEV_MODE
  Serial.println("BME280 initialized!");
#endif

  // ************************
  // || Inititalize Shield ||
  // ************************
  initialize_shield(); // If any external sensors are detected, initialize them

  if (oled_found)
  {
    oled_clearDisplay();
    oled_print_diagnostic("Freq", FSK_FREQ, 3);
    oled_display();
  }
  if (sd_found)
  {
    sd_card_write_line("datalog.csv", "PayloadID,Counter,Hours,Minutes,Seconds,Latitude,Longitude,Altitude,Speed,Sats,Temp,BattVoltage,AscentRate,ExtTemp,Humidity,ExtPress");
  }

  // ******************************
  // || Send Morse Code Callsign ||
  // ******************************

  sendCallsign();

#ifdef DEV_MODE
  Serial.println("Setup done! Beginning control flow.");
#endif

#ifdef STATUS_LED
  digitalWrite(SUCCESS_LED, HIGH);
  delay(1000);
  digitalWrite(SUCCESS_LED, LOW);
#endif

  // *************************
  // || Scheduler Execution ||
  // *************************
  Scheduler.startLoop(gpsFeed);
}

void loop()
{
  // *********************
  // || Local Variables ||
  // *********************
  int coded_len;

  // ***************************
  // || Callsign Transmission ||
  // ***************************

  // Check if it's the right time to send the callsign
  if (call_count * PACKET_INTERVAL >= CALLSIGN_INTERVAL)
  {
    // Send the callsign, and reset the counter
    sendCallsign();
    call_count = 0;
  }

  // ***************************
  // || Generate Horus Packet ||
  // ***************************
#ifdef DEV_MODE
  Serial.println(F("Generating Horus Binary v2 Packet"));
#endif
  coded_len = build_horus_binary_packet_v3((char *)&codedbuffer);

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

// GPS Feed loop to keep the GPS module updated
void gpsFeed()
{
  while (Serial1.available() > 0)
  {
    gps.encode(Serial1.read());
  }
  yield();
}

// Build the Horus v3 Packet. This is where the GPS positions and telemetry are organized to the struct.
size_t build_horus_binary_packet_v3(char *buffer)
{
  static float prev_altitude = 0.0f;
  static unsigned long prev_time = 0;
  float ascent_rate = 0.0f;

  if (prev_time != 0)
  {
    unsigned long time_diff = millis() - prev_time;
    if (time_diff > 0)
    {
      ascent_rate = (gps.altitude.meters() - prev_altitude) / (time_diff / 1000.0f);
    }
  }
  prev_altitude = gps.altitude.meters();
  prev_time = millis();

// Fill with GPS readings, with a GPS sanity check
#ifdef FLAG_BAD_PACKET
  if (gps.altitude.meters() > 0 && gps.altitude.meters() < 50000)
  {
    TelemetryStruct.gps.hours = gps.time.hour();
    TelemetryStruct.gps.minutes = gps.time.minute();
    TelemetryStruct.gps.seconds = gps.time.second();
    TelemetryStruct.gps.latitude = gps.location.lat() * 100000;
    TelemetryStruct.gps.longitude = gps.location.lng() * 100000;
    TelemetryStruct.gps.altitudeMeters = (int)gps.altitude.meters();
    TelemetryStruct.gps.velocityHorizontalKilometersPerHour = (int)gps.speed.kmph();
    TelemetryStruct.gps.ascentRateCentimetersPerSecond = (int)(ascent_rate * 100.0f);
    TelemetryStruct.gps.satellitesVisible = gps.satellites.value();
  }
  else
  {
    prev_time = 0;
    TelemetryStruct.gps.hours = 0;
    TelemetryStruct.gps.minutes = 0;
    TelemetryStruct.gps.seconds = 0;
    TelemetryStruct.gps.latitude = 0;
    TelemetryStruct.gps.longitude = 0;
    TelemetryStruct.gps.altitudeMeters = 0;
    TelemetryStruct.gps.velocityHorizontalKilometersPerHour = 0;
    TelemetryStruct.gps.satellitesVisible = gps.satellites.value();
    TelemetryStruct.gps.ascentRateCentimetersPerSecond = 0;
  }
#else
  // Or, if you prefer no sanity check, force GPS positions into struct
  TelemetryStruct.gps.hours = gps.time.hour();
  TelemetryStruct.gps.minutes = gps.time.minute();
  TelemetryStruct.gps.seconds = gps.time.second();
  TelemetryStruct.gps.latitude = gps.location.lat() * 100000;
  TelemetryStruct.gps.longitude = gps.location.lng() * 100000;
  TelemetryStruct.gps.altitudeMeters = (int)gps.altitude.meters();
  TelemetryStruct.gps.velocityHorizontalKilometersPerHour = (int)gps.speed.kmph();
  TelemetryStruct.gps.ascentRateCentimetersPerSecond = (int)(ascent_rate * 100.0f);
  TelemetryStruct.gps.satellitesVisible = gps.satellites.value();
#endif
#ifdef STATUS_LED
  digitalWrite(SUCCESS_LED, HIGH);
  delay(500);
  digitalWrite(SUCCESS_LED, LOW);
#endif

  // Non-GPS values
  memcpy(TelemetryStruct.callsign, CALLSIGN, sizeof(CALLSIGN));
  TelemetryStruct.sequenceNumber = packet_count;
  TelemetryStruct.bme280.temperature = BME280temperature() / 10;
  TelemetryStruct.bme280.pressure = BME280pressure() / 10;
  TelemetryStruct.bme280.humidity = BME280humidity() / 100;
  TelemetryStruct.batteryMilliVolts = readVoltage() * 1000;

  // Dump the sensor values to Serial Monitor
#ifdef DEV_MODE
  Serial.print("Packet #");
  Serial.print(packet_count);
  Serial.print(": ");
  Serial.print("Lat: ");
  Serial.print(TelemetryStruct.gps.latitude);
  Serial.print(", Lon: ");
  Serial.print(TelemetryStruct.gps.longitude);
  Serial.print(", Alt: ");
  Serial.print(TelemetryStruct.gps.altitudeMeters);
  Serial.print("m, Speed: ");
  Serial.print(TelemetryStruct.gps.velocityHorizontalKilometersPerHour);
  Serial.print("km/h, Ascent Rate: ");
  Serial.print(TelemetryStruct.gps.ascentRateCentimetersPerSecond);
  Serial.print("cm/s, Sats: ");
  Serial.print(TelemetryStruct.gps.satellitesVisible);
  Serial.print(", Temp: ");
  Serial.print(TelemetryStruct.bme280.temperature);
  Serial.print("°C, Pressure: ");
  Serial.print(TelemetryStruct.bme280.pressure);
  Serial.print("hPa, Humidity: ");
  Serial.print(TelemetryStruct.bme280.humidity);
  Serial.print("%, Battery: ");
  Serial.print(TelemetryStruct.batteryMilliVolts);
  Serial.println("mV");
#endif

  // If OLED found, print the values
  if (oled_found)
  {
    oled_clearDisplay();
    oled_setCursor(0, 0);
    oled_print_diagnostic("Sats", TelemetryStruct.gps.satellitesVisible, 0);
    oled_print_diagnostic("Lat", TelemetryStruct.gps.latitude, 6);
    oled_print_diagnostic("Lon", TelemetryStruct.gps.longitude, 6);
    oled_print_diagnostic("Alt", TelemetryStruct.gps.altitudeMeters, 1);
    oled_display();
  }
  if (sd_found)
  {
    /* snprintf(debugbuffer, sizeof(debugbuffer),
             "%u,%u,%u,%u,%u,%.7f,%.7f,%u,%u,%u,%d,%u,%d,%.2f,%u,%u",
             BinaryPacketV2.PayloadID,
             BinaryPacketV2.Counter,
             BinaryPacketV2.Hours,
             BinaryPacketV2.Minutes,
             BinaryPacketV2.Seconds,
             BinaryPacketV2.Latitude,
             BinaryPacketV2.Longitude,
             BinaryPacketV2.Altitude,
             BinaryPacketV2.Speed,
             BinaryPacketV2.Sats,
             BinaryPacketV2.Temp,
             BinaryPacketV2.BattVoltage,
             BinaryPacketV2.AscentRate,
             BinaryPacketV2.ExtTemp / 10,
             BinaryPacketV2.Humidity,
             BinaryPacketV2.ExtPress / 10);
    sd_card_write_line("datalog.csv", debugbuffer); */
  }

  size_t length = radio_horus_v3_encode((uint8_t *)buffer, &TelemetryStruct);

  Serial.println("Encoded packet length: " + String(length) + " bytes");

  return length;
}
