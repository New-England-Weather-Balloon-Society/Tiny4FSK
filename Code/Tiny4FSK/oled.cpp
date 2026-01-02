/*
oled.cpp, part of Tiny4FSK, for a high-altitude tracker.
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

#include "oled.h"
#include "font.h"

// Module-level variables
static int16_t _width;
static int16_t _height;
static uint8_t *buffer = NULL;
static uint8_t textSize = 1;
static uint16_t textColor = 1;
static int16_t cursor_x = 0;
static int16_t cursor_y = 0;

// Forward declarations for local functions
static void sendCommand(uint8_t cmd);
static void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint8_t size);

bool oled_begin(int16_t width, int16_t height, uint8_t i2c_addr)
{
    _width = width;
    _height = height;
    if ((buffer = (uint8_t *)malloc(_width * _height / 8)))
    {
        //Wire.begin();
        memset(buffer, 0, _width * _height / 8);
        sendCommand(0xAE); // Display Off
        sendCommand(0xD5); // Set Display Clock Divide Ratio/Oscillator Frequency
        sendCommand(0x80);
        sendCommand(0xA8); // Set MUX Ratio
        sendCommand(height - 1);
        sendCommand(0xD3); // Set Display Offset
        sendCommand(0x00);
        sendCommand(0x40); // Set Display Start Line
        sendCommand(0x8D); // Charge Pump Setting
        sendCommand(0x14); // Enable Charge Pump
        sendCommand(0x20); // Memory Addressing Mode
        sendCommand(0x00); // Horizontal Addressing Mode
        sendCommand(0xA1); // Set Segment Re-map
        sendCommand(0xC8); // Set COM Output Scan Direction
        sendCommand(0xDA); // Set COM Pins Hardware Configuration
        if (height == 32)
        {
            sendCommand(0x02);
        }
        else
        {
            sendCommand(0x12);
        }
        sendCommand(0x81); // Contrast Control
        sendCommand(0xCF);
        sendCommand(0xD9); // Set Pre-charge Period
        sendCommand(0xF1);
        sendCommand(0xDB); // Set VCOMH Deselect Level
        sendCommand(0x40);
        sendCommand(0xA4); // Display ON
        sendCommand(0xA6); // Normal Display
        sendCommand(0xAF); // Display On
        return true;
    }
    return false;
}

void oled_clearDisplay()
{
    memset(buffer, 0, _width * _height / 8);
}

void oled_display()
{
    sendCommand(0x21); // Set Column Address
    sendCommand(0);
    sendCommand(_width - 1);
    sendCommand(0x22); // Set Page Address
    sendCommand(0);
    sendCommand(_height / 8 - 1);

    uint16_t bufferSize = _width * _height / 8;
    uint8_t wireBufferSize = 32; // BUFFER_LENGTH in Wire.h
    for (uint16_t i = 0; i < bufferSize; i += (wireBufferSize - 1))
    {
        Wire.beginTransmission(SSD1306_I2C_ADDRESS);
        Wire.write(0x40);
        uint16_t end = i + (wireBufferSize - 1);
        if (end > bufferSize)
        {
            end = bufferSize;
        }
        for (uint16_t j = i; j < end; j++)
        {
            Wire.write(buffer[j]);
        }
        Wire.endTransmission();
    }
}

void oled_drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if (x < 0 || x >= _width || y < 0 || y >= _height)
    {
        return;
    }
    switch (color)
    {
    case 1:
        buffer[x + (y / 8) * _width] |= (1 << (y & 7));
        break;
    case 0:
        buffer[x + (y / 8) * _width] &= ~(1 << (y & 7));
        break;
    }
}

void oled_setTextSize(uint8_t s) { textSize = s; }
void oled_setTextColor(uint16_t c) { textColor = c; }
void oled_setCursor(int16_t x, int16_t y)
{
    cursor_x = x;
    cursor_y = y;
}

void oled_print(const char *str)
{
    while (*str)
    {
        drawChar(cursor_x, cursor_y, *str++, textColor, textSize);
        cursor_x += textSize * 6;
        if (cursor_x > _width - textSize * 6)
        {
            cursor_x = 0;
            cursor_y += textSize * 8;
        }
    }
}

void oled_print_diagnostic(const char* name, float value, int decimals) {
    char buf[16];
    char fmt[8];
    snprintf(fmt, sizeof(fmt), "%%.%df", decimals);
    snprintf(buf, sizeof(buf), fmt, value);
    oled_setCursor(0, cursor_y);
    oled_print(name);
    oled_print(": ");
    oled_print(buf);
    cursor_x = 0;
    cursor_y += textSize * 8;
}

static void sendCommand(uint8_t cmd)
{
    Wire.beginTransmission(SSD1306_I2C_ADDRESS);
    Wire.write(0x00); // Co = 0, D/C = 0
    Wire.write(cmd);
    Wire.endTransmission();
}

static void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint8_t size)
{
    if (c < ' ' || c > '~')
    {
        c = '?';
    }
    for (int8_t i = 0; i < 5; i++)
    {
        uint8_t line = font[(c - ' ') * 5 + i];
        for (int8_t j = 0; j < 8; j++)
        {
            if (line & 0x1)
            {
                if (size == 1)
                {
                    oled_drawPixel(x + i, y + j, color);
                }
                else
                {
                    for (int16_t k = 0; k < size; k++)
                    {
                        for (int16_t l = 0; l < size; l++)
                        {
                            oled_drawPixel(x + i * size + k, y + j * size + l, color);
                        }
                    }
                }
            }
            line >>= 1;
        }
    }
}
