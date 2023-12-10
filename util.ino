// Various utility functions
#include "gps.h"
void PrintHex(char *data, uint8_t length, char *tmp){
 // Print char data as hex
 byte first ;
 int j=0;
 for (uint8_t i=0; i<length; i++) 
 {
   first = ((uint8_t)data[i] >> 4) | 48;
   if (first > 57) tmp[j] = first + (byte)39;
   else tmp[j] = first ;
   j++;

   first = ((uint8_t)data[i] & 0x0F) | 48;
   if (first > 57) tmp[j] = first + (byte)39; 
   else tmp[j] = first;
   j++;
 }
 tmp[length*2] = 0;
}

void pulseInt(int pin, int time) {
  digitalWrite(pin, HIGH);
  delay(time);
  digitalWrite(pin, LOW);
}

void gpsSleep(int timeToSleep) {
  // Sleep for 10 seconds
  byte UBLOX_RXM_PMREQ[] = {
    0xB5, 0x62, 0x02, 0x41, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0xF0, 0x38
  };
  sendI2CData(0x42, UBLOX_RXM_PMREQ);
  delayNonBlocking(timeToSleep - 10);
  pulseInt(EXTINT, 20);
}

void sendI2CData(byte address, byte data[]) {
  Wire.beginTransmission(address);  // Start transmission to device
  for (int i = 0; i < sizeof(data); i++) {
    Wire.write(data[i]);  // Send data byte by byte
  }
  Wire.endTransmission();  // End transmission
}

void delayNonBlocking(int timeToDelay) {
  int current = millis();
  int prev = 0;

  if (current - prev >= timeToDelay) {
    prev = current;
  }
}

void sendConfig() {
  sendI2CData(0x42, data1);
  sendI2CData(0x42, data2);
  sendI2CData(0x42, data3);
  sendI2CData(0x42, data4);
  sendI2CData(0x42, data5);
  sendI2CData(0x42, data6);
  sendI2CData(0x42, data7);
  sendI2CData(0x42, data8);
  sendI2CData(0x42, data9);
  sendI2CData(0x42, data10);
  sendI2CData(0x42, data11);
  sendI2CData(0x42, data12);
  sendI2CData(0x42, data13);
  sendI2CData(0x42, data14);
  sendI2CData(0x42, data15);
  sendI2CData(0x42, data16);
  sendI2CData(0x42, data17);
  sendI2CData(0x42, data18);
  sendI2CData(0x42, data19);
  sendI2CData(0x42, data20);
  sendI2CData(0x42, data21);
  sendI2CData(0x42, data22);
  sendI2CData(0x42, data23);
  sendI2CData(0x42, data24);
  sendI2CData(0x42, data25);
}
