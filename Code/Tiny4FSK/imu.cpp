/*
imu.cpp, part of Tiny4FSK, for a high-altitude tracker.
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

#include "imu.h"

// Helper functions
void select_bank(uint8_t bank) {
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(REG_BANK_SEL);
    Wire.write(bank << 4);
    Wire.endTransmission();
}

void write_reg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

uint8_t read_reg(uint8_t reg) {
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(IMU_ADDRESS, (uint8_t)1);
    return Wire.read();
}

bool imu_begin() {
    Wire.begin();
    
    // Select bank 0
    select_bank(0);

    // Check who am I
    if (read_reg(WHO_AM_I) != 0xEA) {
        return false;
    }

    // Reset device
    write_reg(PWR_MGMT_1, 0x80);
    delay(100);

    // Wake up
    write_reg(PWR_MGMT_1, 0x01);
    delay(100);

    // Select bank 2
    select_bank(2);

    // Set gyro sample rate
    write_reg(GYRO_SMPLRT_DIV, 0x07);

    // Set gyro config
    write_reg(GYRO_CONFIG_1, 0x11);

    // Set accel sample rate
    write_reg(ACCEL_SMPLRT_DIV_1, 0x00);
    write_reg(ACCEL_SMPLRT_DIV_2, 0x07);

    // Set accel config
    write_reg(ACCEL_CONFIG, 0x11);

    // Select bank 0
    select_bank(0);

    return true;
}

void imu_read(ImuData* data) {
    select_bank(0);
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(ACCEL_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(IMU_ADDRESS, (uint8_t)6);
    data->accel.x = (Wire.read() << 8) | Wire.read();
    data->accel.y = (Wire.read() << 8) | Wire.read();
    data->accel.z = (Wire.read() << 8) | Wire.read();

    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(GYRO_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(IMU_ADDRESS, (uint8_t)6);
    data->gyro.x = (Wire.read() << 8) | Wire.read();
    data->gyro.y = (Wire.read() << 8) | Wire.read();
    data->gyro.z = (Wire.read() << 8) | Wire.read();
}
