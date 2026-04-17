#include "horus_binary_v3.h"
#include <cstring>

volatile uint16_t horus_v3_packet_counter = 0;
static Telemetry asnMessage;
static BitStream encodedMessage;

int assemble_v3_packet(uint8_t* buffer, telemetry_message* payload) {
    memset(&asnMessage, 0, sizeof(asnMessage));

    memcpy(asnMessage.payloadCallsign, payload->callsign, sizeof(payload->callsign));
    asnMessage.sequenceNumber = constrain(payload->sequenceNumber, 0, 65535);

    asnMessage.timeOfDaySeconds = constrain(payload->gps.hours * 3600 + payload->gps.minutes * 60 + payload->gps.seconds, -1, 86400);
    asnMessage.latitude = constrain(payload->gps.latitude, -9000000, 9000000);
    asnMessage.longitude = constrain(payload->gps.longitude, -18000000, 18000000);
    asnMessage.altitudeMeters = constrain(payload->gps.altitudeMeters, -1000, 50000);
    asnMessage.velocityHorizontalKilometersPerHour = constrain(payload->gps.velocityHorizontalKilometersPerHour, 0, 512);
    asnMessage.ascentRateCentimetersPerSecond = constrain(payload->gps.ascentRateCentimetersPerSecond, -32767, 32767);
    asnMessage.gnssSatellitesVisible = constrain(payload->gps.satellitesVisible, 0, 31);

    asnMessage.humidityPercentage = constrain(payload->bme280.humidity, 0, 100);
    asnMessage.temperatureCelsius_x10.internal = constrain(payload->bme280.temperature, -1023, 1023);
    asnMessage.pressurehPa_x10 = constrain(payload->bme280.pressure, 0, 12000);

    asnMessage.milliVolts.battery = constrain(payload->batteryMilliVolts, 0, 16383);

    asnMessage.exist.velocityHorizontalKilometersPerHour = true;
    asnMessage.exist.ascentRateCentimetersPerSecond = true;
    asnMessage.exist.gnssSatellitesVisible = true;
    asnMessage.temperatureCelsius_x10.exist.internal = true;
    asnMessage.exist.pressurehPa_x10 = true;
    asnMessage.exist.temperatureCelsius_x10 = true;
    asnMessage.exist.humidityPercentage = true;
    asnMessage.milliVolts.exist.battery = true;
    asnMessage.exist.milliVolts = true;

    memset(&encodedMessage, 0, sizeof(encodedMessage));
    int errCode;
    BitStream_Init (&encodedMessage,
                    (unsigned char*)(buffer+2),
                    HORUS_UNCODED_BUFFER_SIZE-2);

    assert_value = 0;

    if(!Telemetry_Encode(&asnMessage, &encodedMessage, &errCode, true) || assert_value != 0) {
        if (errCode != 0) {
            return errCode;
        }
        if (assert_value != 0) {
            return ERR_INCORRECT_PER_STREAM;
        }
    }
    

    int encodedSize = BitStream_GetLength(&encodedMessage);

    int frameSize = 128;
    if (encodedSize <= 30){
        frameSize = 32;
    } else if (encodedSize <= 46){
        frameSize = 48;
    } else if (encodedSize <= 62){
        frameSize = 64;
    } else if (encodedSize <= 94){
        frameSize = 96;
    } else if (encodedSize <= 126){
        frameSize = 128;
    }

    uint16_t crc = (uint16_t)gen_crc16((unsigned char *)(buffer+2), frameSize-2);
    memcpy(buffer, &crc, sizeof(crc));

    return frameSize;
}

uint16_t radio_horus_v3_encode(uint8_t *payload, telemetry_message *telemetry_data)
{
    char horus_packet[HORUS_UNCODED_BUFFER_SIZE];

    memset(horus_packet, 0, HORUS_UNCODED_BUFFER_SIZE);

    int packet_length = assemble_v3_packet((uint8_t *)&horus_packet, telemetry_data);
    if (packet_length <= 0 || packet_length > HORUS_UNCODED_BUFFER_SIZE) {
        Serial.print("ERROR: Horus v3 packet assembly failed with code: ");
        Serial.println(packet_length);
        return 0;
    }

    Serial.print("Horus packet (hex): ");
    for (int i = 0; i < packet_length; i++) {
        if ((uint8_t)horus_packet[i] < 0x10) {
            Serial.print('0');
        }
        Serial.print((uint8_t)horus_packet[i], HEX);
    }
    Serial.println();

    // Preamble to help the decoder lock-on after a quiet period.
    for (int i = 0; i < HORUS_V3_PREAMBLE_LENGTH; i++) {
        payload[i] = HORUS_V3_PREAMBLE_BYTE;
    }

    // Encode the packet, and write into the mfsk buffer.
    int encoded_length = horus_l2_encode_tx_packet(
            (unsigned char *) payload + HORUS_V3_PREAMBLE_LENGTH,
            (unsigned char *) &horus_packet, packet_length);

    if (encoded_length <= 0 || (encoded_length + HORUS_V3_PREAMBLE_LENGTH) > HORUS_CODED_BUFFER_SIZE) {
        Serial.print("ERROR: Horus L2 encoded length invalid: ");
        Serial.println(encoded_length);
        return 0;
    }

    return encoded_length + HORUS_V3_PREAMBLE_LENGTH;
}