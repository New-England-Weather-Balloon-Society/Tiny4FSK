/*
shield.cpp, part of Tiny4FSK, for a high-altitude tracker.
Copyright (C) 2025 Maxwell Kendall

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

// Support interaction with the Tiny4FSK General Shield

#include "shield.h"

bool bme280_found = false;
bool imu_found = false;
bool oled_found = false;
bool sd_found = false;

void initialize_imu()
{
    if (imu_begin())
    {
        Serial.println("IMU Initialized!");
    }
    else
    {
        Serial.println("IMU Init Failed!");
    }
}

void i2c_scan(int *allAddresses)
{
    byte address, error;
    int devices = 0;
    for (int i = 0; i < 127; i++)
    {
        allAddresses[i] = 0;
    }
    for (address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (!error)
        {
            allAddresses[devices] = address;
            devices++;
        }
    }
}

void initialize_shield()
{
    int sensors[127] = {0};
    i2c_scan(sensors);

    Serial.print("I2C addresses found: ");
    for (int i = 0; i < 127; i++)
    {
        if (sensors[i] != 0)
        {
            Serial.print("0x");
            if (sensors[i] < 16)
                Serial.print("0");
            Serial.print(sensors[i], HEX);
            Serial.print(" ");
        }
    }
    Serial.println();

    for (int i = 0; i < (sizeof(sensors) / sizeof(sensors[0])); i++)
    {
        if (sensors[i] == BME_ADDRESS)
        {
            bme280_found = true;
        }
        if (sensors[i] == IMU_ADDRESS)
        {
            imu_found = true;
        }
        if (sensors[i] == 0x3C)
        {
            oled_found = true;
        }
    }

    if (bme280_found)
    {
        // initialize bme!
/*         Serial.println("BME280 found! Initializing...");
        BME280setI2Caddress(BME_ADDRESS);
        BME280setup(); */
    }

    if (imu_found)
    {
        Serial.println("IMU Found! Initializing...");
        //initialize_imu();
    }

    if (oled_found)
    {
        Serial.println("OLED Found! Initializing...");
        oled_begin(128, 32);
        oled_clearDisplay();
        oled_setTextSize(1);
        oled_setTextColor(1);
        oled_setCursor(0, 0);
        oled_print("Tiny4FSK Tracker");
        oled_display();
    }

    if (sd_card_begin()) {
        Serial.println("SD Card Initialized!");
        sd_found = true;
    } else {
        Serial.println("No SD Card Detected...");
    }
}
