/* TinyBME280 Library v2

   David Johnson-Davies - www.technoblogy.com - 22nd June 2019
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>

#pragma once

/* Function declarations */

// Can be called before BME280setup
void BME280setI2Caddress(uint8_t address);

// Sets Normal mode, no upsampling, and reads the chip calibrations
void BME280setup();

// Sets Sleep Mode for sensor
void BME280sleep ();

// Temperature in DegC, resolution is 0.01 DegC
// Output value of “5123” equals 51.23 DegC
int32_t BME280temperature();

// Pressure in Pa as unsigned 32 bit integer
// Output value of “96386” equals 96386 Pa = 963.86 hPa
uint32_t BME280pressure();

// Humidity in %RH, resolution is 0.01%RH
// Output value of “4653” represents 46.53 %RH
uint32_t BME280humidity();