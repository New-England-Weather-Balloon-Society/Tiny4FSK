#pragma once
#include "asn1scc/HorusBinaryV3.h"
#include <Arduino.h>
#include "horus_l2.h"
#define Serial SerialUSB
#define HORUS_V3_PREAMBLE_LENGTH 4
#define HORUS_V3_PREAMBLE_BYTE 0xE4

#define HORUS_UNCODED_BUFFER_SIZE 128
#define HORUS_CODED_BUFFER_SIZE 256

typedef struct telemetry_message {
    char callsign[7];
    uint16_t sequenceNumber;
    struct {
        int latitude; // 5 decimals, x10^5
        int longitude; // 5 decimals, x10^5
        int altitudeMeters;
        uint16_t velocityHorizontalKilometersPerHour;
        int16_t ascentRateCentimetersPerSecond;
        uint8_t satellitesVisible;
        uint8_t hours;
        uint8_t minutes;
        uint8_t seconds;
    } gps;
    struct {
        int16_t temperature; // celsius x10
        int16_t pressure; // hpa x10
        int16_t humidity;
    } bme280;
    uint16_t batteryMilliVolts;
} __attribute__((packed));

int assemble_v3_packet(uint8_t* buffer, telemetry_message* payload);
uint16_t radio_horus_v3_encode(uint8_t *payload, telemetry_message *telemetry_data);
