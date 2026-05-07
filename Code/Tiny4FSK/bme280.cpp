/* TinyBME280 Library v2

   David Johnson-Davies - www.technoblogy.com - 22nd June 2019
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#include "bme280.h"

int16_t T[4], P[10], H[7];
int32_t BME280t_fine;

int BME280address = 118;

int16_t read16 () {
  uint8_t lo, hi;
  lo = Wire.read(); hi = Wire.read();
  return hi<<8 | lo;
}

int32_t read32 () {
  uint8_t msb, lsb, xlsb;
  msb = Wire.read(); lsb = Wire.read(); xlsb = Wire.read();
  return (uint32_t)msb<<12 | (uint32_t)lsb<<4 | (xlsb>>4 & 0x0F);
}

// Can be called before BME280setup
void BME280setI2Caddress (uint8_t address) {
  BME280address = address;
}

// Must be called once at start
void BME280setup () {
  delay(2);
  // Set the mode to Normal, no upsampling
  Wire.beginTransmission(BME280address);
  Wire.write(0xF2);                             // ctrl_hum
  Wire.write(0b00000001);
  Wire.write(0xF4);                             // ctrl_meas
  Wire.write(0b00100111);
  // Read the chip calibrations.
  Wire.write(0x88);
  Wire.endTransmission();
  Wire.requestFrom(BME280address, 26);
  for (int i=1; i<=3; i++) T[i] = read16();     // Temperature
  for (int i=1; i<=9; i++) P[i] = read16();     // Pressure
  Wire.read();  // Skip 0xA0
  H[1] = (uint8_t)Wire.read();                  // Humidity
  //
  Wire.beginTransmission(BME280address);
  Wire.write(0xE1);
  Wire.endTransmission();
  Wire.requestFrom(BME280address, 7);
  H[2] = read16();
  H[3] = (uint8_t)Wire.read();
  uint8_t e4 = Wire.read(); uint8_t e5 = Wire.read();
  H[4] = ((int16_t)((e4 << 4) + (e5 & 0x0F)));
  H[5] = ((int16_t)((Wire.read() << 4) + ((e5 >> 4) & 0x0F)));
  H[6] = ((int8_t)Wire.read()); // 0xE7
  // Read the temperature to set BME280t_fine
  BME280temperature();
}

// Can be called if sensor can sleep to save energy
void BME280sleep () {
  delay(2);
  // Set the mode to Normal, no upsampling
  Wire.beginTransmission(BME280address);
  Wire.write(0xF4);                             // ctrl_meas
  Wire.write(0b00000000);                       // what can be better than sleep?!
  Wire.endTransmission();
}

// Returns temperature in DegC, resolution is 0.01 DegC
// Output value of “5123” equals 51.23 DegC
int32_t BME280temperature () {
  Wire.beginTransmission(BME280address);
  Wire.write(0xFA);
  Wire.endTransmission();
  Wire.requestFrom(BME280address, 3);
  int32_t adc = read32();
  // Compensate
  int32_t var1, var2, t; 
  var1 = ((((adc>>3) - ((int32_t)((uint16_t)T[1])<<1))) * ((int32_t)T[2])) >> 11;
  var2 = ((((adc>>4) - ((int32_t)((uint16_t)T[1]))) * ((adc>>4) - ((int32_t)((uint16_t)T[1])))) >> 12);
  var2 = (var2 * ((int32_t)T[3])) >> 14;
  BME280t_fine = var1 + var2;
  return (BME280t_fine*5+128)>>8;
}

// Returns pressure in Pa as unsigned 32 bit integer
// Output value of “96386” equals 96386 Pa = 963.86 hPa
uint32_t BME280pressure () {
  Wire.beginTransmission(BME280address);
  Wire.write(0xF7);
  Wire.endTransmission();
  Wire.requestFrom(BME280address, 3);
  int32_t adc = read32();
  // Compensate
  int32_t var1, var2;
  uint32_t p;
  var1 = (((int32_t)BME280t_fine)>>1) - (int32_t)64000;
  var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)P[6]);
  var2 = var2 + ((var1*((int32_t)P[5]))<<1);
  var2 = (var2>>2) + (((int32_t)P[4])<<16);
  var1 = (((P[3] * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)P[2]) * var1)>>1))>>18;
  var1 = ((((32768+var1))*((int32_t)((uint16_t)P[1])))>>15);
  if (var1 == 0) return 0;
  p = (((uint32_t)(((int32_t)1048576) - adc) - (var2>>12)))*3125;
  if (p < 0x80000000) p = (p << 1) / ((uint32_t)var1);
  else p = (p / (uint32_t)var1) * 2;
  var1 = (((int32_t)P[9]) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
  var2 = (((int32_t)(p>>2)) * ((int32_t)P[8]))>>13;
  p = (uint32_t)((int32_t)p + ((var1 + var2 + P[7]) >> 4));
  return p;
}

// Humidity in %RH, resolution is 0.01%RH
// Output value of “4653” represents 46.53 %RH
uint32_t BME280humidity () {
  Wire.beginTransmission(BME280address);
  Wire.write(0xFD);
  Wire.endTransmission();
  Wire.requestFrom(BME280address, 2);
  uint8_t hi = Wire.read(); uint8_t lo = Wire.read();
  int32_t adc = (uint16_t)(hi<<8 | lo);
  // Compensate
  int32_t var1; 
  var1 = (BME280t_fine - ((int32_t)76800));
  var1 = (((((adc << 14) - (((int32_t)H[4]) << 20) - (((int32_t)H[5]) * var1)) +
  ((int32_t)16384)) >> 15) * (((((((var1 * ((int32_t)H[6])) >> 10) * (((var1 *
  ((int32_t)H[3])) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
  ((int32_t)H[2]) + 8192) >> 14));
  var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)H[1])) >> 4));
  var1 = (var1 < 0 ? 0 : var1);
  var1 = (var1 > 419430400 ? 419430400 : var1);
  return (uint32_t)((var1>>12)*25)>>8;
}