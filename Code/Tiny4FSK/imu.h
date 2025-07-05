/*
imu.h, part of Tiny4FSK, for a high-altitude tracker.
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

#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

// ICM-20948 Registers
#define REG_BANK_SEL 0x7F

// Bank 0
#define WHO_AM_I 0x00
#define USER_CTRL 0x03
#define PWR_MGMT_1 0x06
#define PWR_MGMT_2 0x07
#define ACCEL_XOUT_H 0x2D
#define GYRO_XOUT_H 0x33

// Bank 2
#define GYRO_SMPLRT_DIV 0x00
#define GYRO_CONFIG_1 0x01
#define ACCEL_SMPLRT_DIV_1 0x10
#define ACCEL_SMPLRT_DIV_2 0x11
#define ACCEL_CONFIG 0x14


bool imu_begin();

struct ImuValues {
    int16_t x;
    int16_t y;
    int16_t z;
};

struct ImuData {
    ImuValues accel;
    ImuValues gyro;
};

void imu_read(ImuData* data);

