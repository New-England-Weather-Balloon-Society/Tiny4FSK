// Various utility functions

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
