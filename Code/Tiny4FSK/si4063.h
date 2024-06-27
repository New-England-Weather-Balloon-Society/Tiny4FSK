// Basic Si4063 implementation - MINIMAL DRIVER

#pragma once

#include <stdint.h>

extern unsigned int SI4063_clock;
extern unsigned int NSEL;
extern unsigned int SDN;

typedef enum _HAL_response
{
    HAL_OK = 0,
    HAL_ERROR = -1,
    HAL_ERROR_TIMEOUT = -2,
} HAL_response;

typedef enum _si4063_command
{
    SI4063_COMMAND_PART_INFO = 0x01,
    SI4063_COMMAND_POWER_UP = 0x02,
    SI4063_COMMAND_SET_PROPERTY = 0x11,
    SI4063_COMMAND_GPIO_PIN_CFG = 0x13,
    SI4063_COMMAND_FIFO_INFO = 0x15,
    SI4063_COMMAND_GET_INT_STATUS = 0x20,
    SI4063_COMMAND_START_TX = 0x31,
    SI4063_COMMAND_REQUEST_DEVICE_STATE = 0x33,
    SI4063_COMMAND_CHANGE_STATE = 0x34,
    SI4063_COMMAND_GET_ADC_READING = 0x14,
    SI4063_COMMAND_READ_CMD_BUFF = 0x44,
    SI4063_COMMAND_WRITE_TX_FIFO = 0x66,
} si4063_command;

typedef enum _si4063_state
{
    SI4063_STATE_SLEEP = 0x01,
    SI4063_STATE_SPI_ACTIVE = 0x02,
    SI4063_STATE_READY = 0x03,
    SI4063_STATE_READY2 = 0x04,
    SI4063_STATE_TX_TUNE = 0x05,
    SI4063_STATE_TX = 0x07,
} si4063_state;

typedef enum _si4063_modulation_type
{
    SI4063_MODULATION_TYPE_CW = 0,
    SI4063_MODULATION_TYPE_OOK,
    SI4063_MODULATION_TYPE_FSK,
    SI4063_MODULATION_TYPE_FIFO_FSK,
} si4063_modulation_type;

struct chip_parameters
{
    uint8_t gpio0;
    uint8_t gpio1;
    uint8_t gpio2;
    uint8_t gpio3;
    uint8_t drive_strength;
    unsigned int clock;
} __attribute__((packed));

struct radio_parameters
{
    uint32_t frequency_hz;
    uint32_t rate_bps;
    uint32_t data_rate;
    uint8_t power;
    si4063_modulation_type type;
    uint8_t offset;
    uint32_t deviation_hz;
} __attribute__((packed));

int si4063_power_up();
int si4063_init(radio_parameters rp, chip_parameters cp);
void si4063_configure_chip(chip_parameters chip_params);
void si4063_configure_rf(radio_parameters params);

void si4063_configure_data_rate(uint32_t data_rate);
void si4063_set_frequency_offset(uint16_t offset);
void si4063_set_frequency_deviation(uint32_t deviation_hz);
void si4063_set_modulation_type(si4063_modulation_type type);
void si4063_set_data_rate(const uint32_t rate_bps);
void si4063_set_tx_power(uint8_t power);
void si4063_set_tx_frequency(const uint32_t frequency_hz);
int si4063_set_clock(unsigned int clock);

int si4063_get_outdiv(const uint32_t frequency_hz);
int si4063_get_band(const uint32_t frequency_hz);

void si4063_enable_tx();
void si4063_inhibit_tx();
void si4063_disable_tx();
uint16_t si4063_start_tx(uint8_t *data, int len);
int si4063_wait_for_tx_complete(int timeout_ms);
bool si4063_fifo_underflow();

uint32_t si4063_calculate_deviation(uint32_t deviation_hz);
uint16_t si4063_read_part_info();

void si4063_set_state(si4063_state state);
int si4063_wait_for_cts();
void si4063_send_command(si4063_command command, uint8_t length, uint8_t *data);
int si4063_read_response(uint8_t length, uint8_t *data);
uint8_t spi_read();