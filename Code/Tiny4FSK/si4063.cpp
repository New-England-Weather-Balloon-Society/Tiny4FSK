/*
si4063.cpp, part of Tiny4FSK, for a high-altitude tracker.
Copyright (C) 2024 Maxwell Kendall

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

#include "si4063.h"

uint32_t current_frequency_hz = 434000000UL;
uint32_t current_deviation_hz = 0;

unsigned int SI4063_clock = 26000000UL;
unsigned int NSEL = NSEL_PIN;
unsigned int SDN = SDN_PIN;

int si4063_power_up()
{
  si4063_wait_for_cts();

  uint8_t data[] = {
      0x01, 0x01, 0x01, 0x8C, 0xBA, 0x80};

  si4063_send_command(SI4063_COMMAND_POWER_UP, sizeof(data), data);

  return si4063_wait_for_cts();
}

int si4063_init(radio_parameters rp, chip_parameters cp)
{
  // SPI.begin();

  digitalWrite(SDN, LOW);
  delayMicroseconds(50);

  si4063_wait_for_cts();

  digitalWrite(SDN, HIGH);
  delayMicroseconds(20);
  digitalWrite(SDN, LOW);
  delayMicroseconds(50);

  SPI.begin();

  if (si4063_power_up() != HAL_OK)
  {
    Serial.println("ERROR: Error powering up Si4063\n");
    return HAL_ERROR;
  }

  // Assume Si4063 part number
  uint16_t part = si4063_read_part_info();
  // if (part != 0x4063) {
  // Serial.print("ERROR: Unknown or missing Si4063 part number: ");
  // Serial.println(part, HEX);
  // return HAL_ERROR;
  //}
  Serial.print("Detected part number: 0x");
  Serial.println(part, HEX);

  si4063_configure_rf(rp);

  si4063_configure_chip(cp);

  return HAL_OK;
}

void si4063_configure_chip(chip_parameters chip_params)
{
  uint8_t data[] = {
      chip_params.gpio0,
      chip_params.gpio1,
      chip_params.gpio2,
      chip_params.gpio3,
      0x00, // NIRQ = Do nothing
      11,   // SDO 11 = Outputs the Serial Data Out (SDO) signal for the SPI bus
      chip_params.drive_strength};

  si4063_send_command(SI4063_COMMAND_GPIO_PIN_CFG, sizeof(data), data);

  si4063_set_clock(chip_params.clock);
}

void si4063_configure_rf(radio_parameters params)
{
  {
    uint8_t data[] = {
        0x00, // 0x00 = Group GLOBAL
        0x01, // Set 1 property
        0x00, // 0x00 = GLOBAL_XO_TUNE
        0x62  // Value determined for DFM17 radiosondes
    };

    si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);
  }

  {
    uint8_t data[] = {
        0x00, // 0x00 = Group GLOBAL
        0x01, // Set 1 property
        0x01, // 0x00 = GLOBAL_CLK_CFG
        0x00  // No clock output needed
    };

    si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);
  }

  {
    uint8_t data[] = {
        0x00,      // 0x00 = Group GLOBAL
        0x01,      // Set 1 property
        0x03,      // 0x03 = GLOBAL_CONFIG
        0x40 |     // 0x40 = Reserved, needs to be set to 1
            0x20 | // 0x20 = Fast sequencer mode
            0x10 | // 129-byte FIFO
            0x00   // High-performance mode
    };

    si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);
  }

  {
    uint8_t data[] = {
        0x01, // 0x01 = Group INT_CTL
        0x01, // Set 1 property
        0x00, // 0x00 = INT_CTL_ENABLE
        0x00  // 0x00 = Disable all hardware interrupts
    };

    si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);
  }

  {
    uint8_t data[] = {
        0x02, // 0x02 = Group FRR_CTL
        0x04, // Set 4 properties
        0x00, // 0x00 = FRR_CTL_A_MODE
        0x00, // Disable all FRR values
        0x00,
        0x00,
        0x00};

    si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);
  }

  {
    // Used only in synchronous mode (for GFSK modulation/filtering)
    uint8_t data[] = {
        0x10, // 0x10 = Group PREAMBLE
        0x01, // Set 1 property
        0x00, // 0x00 = PREAMBLE_TX_LENGTH
        0x00  // 0x00 = Disable preamble
    };

    si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);
  }

  {
    // Used only in synchronous mode (for GFSK modulation/filtering)
    uint8_t data[] = {
        0x11, // 0x11 = Group SYNC
        0x01, // Set 1 property
        0x00, // 0x00 = SYNC_CONFIG
        0x80  // 0x80 = Sync word is not transmitted
    };

    si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);
  }

  {
    uint8_t data[] = {
        0x22, // 0x22 = Group PA
        0x01, // Set 1 property
        0x02, // 0x02 = PA_BIAS_CLKDUTY
        0x00  // 0x00 = Complementary drive signals, 50% duty cycle. For high-power applications.
              // Alternative: 0xC0 = Single-ended drive signal, 25% duty cycle. For low-power applications.
    };

    si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);
  }

  {
    /*
          2. Detailed Errata Descriptions
2.1 If Configured to Skip Sync and Preamble on Transmit, the TX Data from the FIFO is Corrupted
Description of Errata
If preamble and sync word are excluded from the transmitted data (PREMABLE_TX_LENGTH = 0 and SYNC_CONFIG: SKIP_TX = 1), data from the FIFO is not transmitted correctly.
Affected Conditions / Impacts
Some number of missed bytes will occur at the beginning of the packet and some number of repeated bytes at the end of the packet.
Workaround
Set PKT_FIELD_1_CRC_CONFIG: CRC_START to 1. This will trigger the packet handler and result in transmitting the correct data,
while still not sending a CRC unless enabled in a FIELD configuration. A fix has been identified and will be included in a future release
        */

    // In other words, without this, the FIFO buffer gets corrupted while TXing, because we're not using
    // the preamble/sync word stuff
    // To be clear - we're not doing any CRC stuff! This is just the recommended workaround
    uint8_t data[] = {
        0x12, // Group
        0x01, // Set 1 property
        0x10, // PKT_FIELD_1_CRC_CONFIG
        0x80, // CRC_START
    };
    si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);
  }

  si4063_configure_data_rate(params.data_rate);
  si4063_set_frequency_offset(params.offset);
  si4063_set_frequency_deviation(params.deviation_hz);
  si4063_set_modulation_type(params.type);
  si4063_set_data_rate(params.rate_bps);
  si4063_set_tx_power(params.power);
  si4063_set_tx_frequency(params.frequency_hz);
}

void si4063_configure_data_rate(uint32_t data_rate)
{
  // Used only for GFSK mode filtering
  uint8_t data[] = {
      0x20, // 0x20 = Group MODEM
      0x03, // Set 3 properties
      0x03, // 0x03 = MODEM_DATA_RATE
      (data_rate >> 16) & 0xFF,
      (data_rate >> 8) & 0xFF,
      data_rate & 0xFF};

  si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);
}

void si4063_set_frequency_offset(uint16_t offset)
{
  uint8_t data[] = {
      0x20,         // 0x20 = Group MODEM
      0x02,         // Set 2 properties (2 bytes)
      0x0D,         // 0x0D = MODEM_FREQ_OFFSET
      offset >> 8,  // Upper 8 bits of the offset
      offset & 0xFF // Lower 8 bits of the offset
  };

  si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);
}

void si4063_set_frequency_deviation(uint32_t deviation_hz)
{
  uint32_t deviation = si4063_calculate_deviation(deviation_hz);

  uint8_t data[] = {
      0x20, // 0x20 = Group MODEM
      0x03, // Set 3 properties (3 bytes)
      0x0A, // 0x0A = MODEM_FREQ_DEV
      (deviation >> 16) & 0xFF,
      (deviation >> 8) & 0xFF,
      deviation & 0xFF};

  Serial.print("Si4063: Set frequency deviation to value ");
  Serial.print(deviation);
  Serial.print(" with ");
  Serial.println(deviation_hz);

  si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);

  current_deviation_hz = deviation_hz;
}

void si4063_set_modulation_type(si4063_modulation_type type)
{
  uint8_t data[] = {
      0x20,      // 0x20 = Group MODEM
      0x01,      // Set 1 property
      0x00,      // 0x00 = MODEM_MOD_TYPE
      0x80 |     // 0x80 = Direct async mode (MCU-controlled)
          0x60 | // 0x60 = Use GPIO3 as source for direct mode modulation
          0x08   // 0x08 = Direct modulation source (MCU-controlled)
  };

  Serial.print("Si4063: Set modulation type ");
  Serial.println(type);

  switch (type)
  {
  case SI4063_MODULATION_TYPE_CW:
    // Pure carrier wave modulation (for modulating via frequency offset, e.g. for RTTY)
    data[3] |= 0x00;
    break;
  case SI4063_MODULATION_TYPE_OOK:
    // Direct Async Mode with OOK modulation
    data[3] |= 0x01;
    break;
  case SI4063_MODULATION_TYPE_FSK:
    // Direct Async Mode with FSK modulation
    data[3] |= 0x02;
    break;
  case SI4063_MODULATION_TYPE_FIFO_FSK:
    // FIFO with FSK modulation
    data[3] = 0x02;
    break;
  default:
    return;
  }

  si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);
}

void si4063_set_data_rate(const uint32_t rate_bps)
{
  int rate = rate_bps * 10;
  // Set TX_NCO_MODE to our crystal frequency, as recommended by the data sheet for rates <= 200kbps
  // Set MODEM_DATA_RATE to rate_bps * 10 (will get downsampled because NCO_MODE defaults to 10x)
  uint8_t data[] = {
      0x20, // Group
      0x07, // Set 7 properties
      0x03, // Start from MODEM_DATA_RATE
      (rate >> 16) & 0xFF,
      (rate >> 8) & 0xFF,
      rate & 0xFF,
      (SI4063_clock >> 24) & 0xFF,
      (SI4063_clock >> 16) & 0xFF,
      (SI4063_clock >> 8) & 0xFF,
      SI4063_clock & 0xFF,
  };
  si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);
}

void si4063_set_tx_power(uint8_t power)
{
  uint8_t data[] = {
      0x22,        // 0x20 = Group PA
      0x01,        // Set 1 property
      0x01,        // 0x01 = PA_PWR_LVL
      power & 0x7F // Power level from 00..7F
  };

  Serial.print("Si4063: Set TX power ");
  Serial.println(power);

  si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);
}

void si4063_set_tx_frequency(const uint32_t frequency_hz)
{
  uint8_t outdiv, band;
  uint32_t f_pfd, n, m;
  float ratio, rest;

  Serial.print("Si4063: Set frequency ");
  Serial.println(frequency_hz);

  outdiv = si4063_get_outdiv(frequency_hz);
  band = si4063_get_band(frequency_hz);

  f_pfd = 2 * SI4063_clock / outdiv;
  n = frequency_hz / f_pfd - 1;

  ratio = (float)frequency_hz / f_pfd;
  rest = ratio - n;

  m = rest * 524288UL;

  // Set the frequency band
  {
    uint8_t data[] = {
        0x20,       // 0x20 = Group MODEM
        0x01,       // Set 1 property
        0x51,       // 0x51 = MODEM_CLKGEN_BAND
        0x08 + band // 0x08 = SY_SEL: High Performance mode (fixed prescaler = Div-by-2). Finer tuning.
    };

    si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);
  }

  // Set the PLL parameters
  {
    uint8_t data[] = {
        0x40,             // 0x40 = Group FREQ_CONTROL
        0x06,             // Set 6 properties
        0x00,             // 0x00 = Start from FREQ_CONTROL_INTE
        n,                // 0 (FREQ_CONTROL_INTE): Frac-N PLL Synthesizer integer divide number.
        (m >> 16) & 0xFF, // 1 (FREQ_CONTROL_FRAC): Frac-N PLL fraction number.
        (m >> 8) & 0xFF,  // 2 (FREQ_CONTROL_FRAC): Frac-N PLL fraction number.
        m & 0xFF,         // 3 (FREQ_CONTROL_FRAC): Frac-N PLL fraction number.
        0x00,             // 4 (FREQ_CONTROL_CHANNEL_STEP_SIZE): EZ Frequency Programming channel step size.
        0x02              // 5 (FREQ_CONTROL_CHANNEL_STEP_SIZE): EZ Frequency Programming channel step size.
    };

    si4063_send_command(SI4063_COMMAND_SET_PROPERTY, sizeof(data), data);
  }

  current_frequency_hz = frequency_hz;

  // Deviation depends on the frequency band
  si4063_set_frequency_deviation(current_deviation_hz);
}

int si4063_set_clock(unsigned int clock)
{
  if (clock > 26000000UL && clock < 30000000UL)
  {
    SI4063_clock = clock;
    return HAL_OK;
  }
  return HAL_ERROR;
}

int si4063_get_outdiv(const uint32_t frequency_hz)
{
  // Select the output divider according to the recommended ranges in the Si406x datasheet
  if (frequency_hz < 177000000UL)
  {
    return 24;
  }
  else if (frequency_hz < 239000000UL)
  {
    return 16;
  }
  else if (frequency_hz < 353000000UL)
  {
    return 12;
  }
  else if (frequency_hz < 525000000UL)
  {
    return 8;
  }
  else if (frequency_hz < 705000000UL)
  {
    return 6;
  }

  return 4;
}

int si4063_get_band(const uint32_t frequency_hz)
{
  if (frequency_hz < 177000000UL)
  {
    return 5;
  }
  else if (frequency_hz < 239000000UL)
  {
    return 4;
  }
  else if (frequency_hz < 353000000UL)
  {
    return 3;
  }
  else if (frequency_hz < 525000000UL)
  {
    return 2;
  }
  else if (frequency_hz < 705000000UL)
  {
    return 1;
  }

  return 0;
}

void si4063_enable_tx()
{
  si4063_set_state(SI4063_STATE_TX);
}

void si4063_inhibit_tx()
{
  si4063_set_state(SI4063_STATE_READY);
}

void si4063_disable_tx()
{
  // Is this needed?
  si4063_set_state(SI4063_STATE_SLEEP);
}

// Returns number of bytes sent from *data
// If less than len, remaining bytes will need to be used to top up the buffer
uint16_t si4063_start_tx(uint8_t *data, int len)
{
  // Clear fifo underflow interrupt
  si4063_fifo_underflow();

  // Clear TX FIFO
  uint8_t fifo_clear_data[] = {1};
  si4063_send_command(SI4063_COMMAND_FIFO_INFO, 1, fifo_clear_data);
  si4063_wait_for_cts();

  // Add our data to the TX FIFO
  int fifo_len = len;
  if (fifo_len > 64)
  {
    fifo_len = 64;
  }
  si4063_send_command(SI4063_COMMAND_WRITE_TX_FIFO, fifo_len, data);

  // Start transmitting
  uint8_t tx_cmd[] = {
      0, // channel
      SI4063_STATE_SLEEP << 4,
      len >> 8,
      len & 0xFF,
      0 // delay
  };
  si4063_send_command(SI4063_COMMAND_START_TX, sizeof(tx_cmd), tx_cmd);
  si4063_wait_for_cts();

  return fifo_len;
}

int si4063_wait_for_tx_complete(int timeout_ms)
{
  si4063_fifo_underflow();
  si4063_send_command(SI4063_COMMAND_GET_INT_STATUS, 1, 0);
  si4063_wait_for_cts();
  for (int i = 0; i < timeout_ms; i++)
  {
    uint8_t status = 0;
    si4063_send_command(SI4063_COMMAND_REQUEST_DEVICE_STATE, 0, NULL);
    si4063_read_response(1, &status);

    if (status == SI4063_STATE_SLEEP || status == SI4063_STATE_READY || status == SI4063_STATE_READY2 || status == SI4063_STATE_SPI_ACTIVE)
    {
      return HAL_OK;
    }

    delay(1);
  }

  si4063_wait_for_cts();
  si4063_disable_tx();
  si4063_wait_for_cts();

  return HAL_ERROR_TIMEOUT;
}

bool si4063_fifo_underflow()
{
  uint8_t data[] = {0xFF, 0xFF, ~0x20}; // Clear underflow status
  si4063_send_command(SI4063_COMMAND_GET_INT_STATUS, sizeof(data), data);
  uint8_t response[7];
  si4063_read_response(sizeof(response), response);

  bool fifo_underflow_pending = response[6] & 0x20;
  return fifo_underflow_pending;
}

uint32_t si4063_calculate_deviation(uint32_t deviation_hz)
{
  uint8_t outdiv = si4063_get_outdiv(current_frequency_hz);

  // SY_SEL = Div-by-2
  return (uint32_t)(((double)(1 << 19) * outdiv * deviation_hz) / (2 * SI4063_clock));
}

uint16_t si4063_read_part_info()
{
  uint8_t response[8];

  si4063_send_command(SI4063_COMMAND_PART_INFO, 0, NULL);

  si4063_read_response(sizeof(response), response);

  // Return part number
  return response[1] << 8 | response[2];
}

void si4063_set_state(si4063_state state)
{
  si4063_send_command(SI4063_COMMAND_CHANGE_STATE, 1, (uint8_t *)&state);
}

int si4063_wait_for_cts()
{
  //SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  uint16_t timeout = 0xFFFF;
  uint8_t response;

  // Poll CTS over SPI
  do
  {
    digitalWrite(NSEL, LOW);
    SPI.transfer(SI4063_COMMAND_READ_CMD_BUFF);
    response = spi_read();
    digitalWrite(NSEL, HIGH);
  } while (response != 0xFF && timeout--);

  if (timeout == 0)
  {
    ;
  }
  //SPI.endTransaction();
  return timeout > 0 ? HAL_OK : HAL_ERROR;
}

void si4063_send_command(si4063_command command, uint8_t length, uint8_t *data)
{
  si4063_wait_for_cts();

  //SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  digitalWrite(NSEL, LOW);

  SPI.transfer(command);

  while (length--)
  {
    SPI.transfer(*(data++));
  }

  digitalWrite(NSEL, HIGH);
  //SPI.endTransaction();
}

int si4063_read_response(uint8_t length, uint8_t *data)
{
  //SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  uint16_t timeout = 0xFFFF;
  uint8_t response;

  // Poll CTS over SPI
  do
  {
    digitalWrite(NSEL, LOW);
    SPI.transfer(SI4063_COMMAND_READ_CMD_BUFF);
    response = spi_read();
    if (response == 0xFF)
    {
      break;
    }
    digitalWrite(NSEL, HIGH);

    delayMicroseconds(10);
  } while (timeout--);

  if (timeout == 0)
  {
    digitalWrite(NSEL, HIGH);
    return HAL_ERROR;
  }

  // Read the requested data
  while (length--)
  {
    *(data++) = spi_read();
  }

  digitalWrite(NSEL, HIGH);
  //SPI.endTransaction();

  return HAL_OK;
}

uint8_t spi_read()
{
  return SPI.transfer(0x00);
}