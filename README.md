# Horus Binary 4FSK Transmit Example, using RadioLib

This repository contains a worked example of generating and transmitting [Horus Binary](https://github.com/projecthorus/horusdemodlib/wiki) v1 and v2 high-altitude balloon telemetry packets on an Arduino-compatible platform, using [RadioLib](https://github.com/jgromes/RadioLib).

This is not a complete high-altitude-balloon tracker codebase, just an example of generating and transmitting Horus Binary packets, for integration into other codebases.

This example uses a Semtech SX1278-compatible radio module (in my case, a HopeRF RFM98W), connected to an Arduino-compatible microcontroller (I used a Seeduino Mega 2560 because it would do 3.3v logic levels). In my case, the module has the usual SPI connections, and the following other pins are used:
* NSS pin:   10
* DIO0 pin:  2
* RESET pin: 9
* DIO1 pin:  3


### Contacts
* Mark Jessop <vk5qi@rfhead.net>

## Dependencies
* Arduino IDE (or other compiler that can compile Arduino projects)
* RadioLib Library - https://github.com/jgromes/RadioLib

## Compiling this Example
It should be possible to just open the horusbinary_radiolib.ino file in the Arduino IDE, select your target platform, and compile/flash. 

Note that I am using some AVR-specific CRC16 libraries (`util/crc16.h`), which will probably need to be replaced for use on other platforms (refer `util.ino`).

## TODO List
* Split out Horus-specific code into separate files, for easier integration into other codebases.