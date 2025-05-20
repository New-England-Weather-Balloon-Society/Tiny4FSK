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

#include "shield.h"

// Support interaction with the Tiny4FSK General Shield
int* i2c_scan() {
    byte address, error;
    int devices = 0;
    int allAddresses[127] = {0};
    for (address=1; address<127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if(!error) {
            Serial.print("Device found at 0x");
            allAddresses[devices] = address;
            if(address < 16) {
                Serial.print("0");
            }
            Serial.println(address, HEX);
            devices++;
        } else if(error==4) {
            Serial.print("Error at address 0x");
            if(address < 16) {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }
    }
    !devices ? Serial.println("No devices found on I2C") : Serial.println("done i2c scan");
    return allAddresses;
}

void initialize_shield() {
    int* sensors = i2c_scan();
    bool bme280_found = false;
    bool imu_found = false;
    bool oled_found = false;

    for(int i=0; i < sizeof(sensors); i++) {
        if(sensors[i] == BME_ADDRESS) {
            bme280_found = true;
        } else if(sensors[i] == IMU_ADDRESS) {
            imu_found = true;
        } else if(sensors[i] == 0xC3) {
            oled_found = true;
        }
    }

    if(bme280_found) {
        // initialize bme!
        BME280setI2Caddress(BME_ADDRESS);
        BME280setup();
        // cooked
    }

    if(imu_found) {
        // initialize imu!airtable automation please run to update my total time
        //maybe in 5 hours?
        //idk i'm done for todayy...
    }

    if(oled_found) {
        // i have never worked with an oled
    }
}