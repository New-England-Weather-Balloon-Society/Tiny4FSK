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

## Horus Binary Generation & Transmission Overview

### Packet Structure
The Horus Binary [packet formats](https://github.com/projecthorus/horusdemodlib/wiki/4-Packet-Format-Details) are represented in this codebase as structs located [here](https://github.com/projecthorus/horusbinary_radiolib/blob/main/horusbinary_radiolib.ino#L45) (v1) and [here](https://github.com/projecthorus/horusbinary_radiolib/blob/main/horusbinary_radiolib.ino#L63) (v2). The last 9 bytes (before the CRC16 checksum) of the v2 packet format are customisable by the user ([refer here](https://github.com/projecthorus/horusdemodlib/wiki/5-Customising-a-Horus-Binary-v2-Packet) for more info).

### Generating a Packet
The two functions [`build_horus_binary_packet_v1`](https://github.com/projecthorus/horusbinary_radiolib/blob/main/horusbinary_radiolib.ino#L94) and [`build_horus_binary_packet_v2`](https://github.com/projecthorus/horusbinary_radiolib/blob/main/horusbinary_radiolib.ino#L121) instantiate a struct, and write data into it. The struct bytes are then written out to a buffer (in this example, `rawbuffer`) supplied as a function argument, and the function returns the length of the packet in bytes.

```c
int pkt_len = build_horus_binary_packet_v1(rawbuffer);
```

### Encoding the Packet
Next, we need to apply the Golay(23,12) forward-error-correction, interleaving and scrambling which protect the telemetry data. This is performed using the `horus_l2_encode_packet` function, which is located in `horus_l2.cpp`. This function takes a pointer to a destination and source buffer, and the packet length:

```c
int coded_len = horus_l2_encode_tx_packet((unsigned char*)codedbuffer,(unsigned char*)rawbuffer,pkt_len);
```

At this stage we now have the encoded packet in `codedbuffer`, and the length of the encoded packet in `coded_len`.

### Transmitting the Packet
In this example codebase, we are using RadioLib to provide a nice abstracted interface to a range of radio transmitter modules. In my case I'm using a SX127x-compatible module, which is started as follows:

```c
// SX1278 has the following connections:
// NSS pin:   10
// DIO0 pin:  2
// RESET pin: 9
// DIO1 pin:  3
SX1278 radio = new Module(10, 2, 9, 3);
```

The radio has to be initialised in FSK mode:
```c
int state = radio.beginFSK();
```

Next, I have provided some functions in `FSK4_Mod.ino` which will make use of the radio modules 'transmitDirect' functionality to transmit the 4FSK modulation. We have to initialise these functions using the fsk4_setup function:

```c
state = fsk4_setup(&radio, TX_FREQ, FSK4_SPACING, FSK4_BAUD);
```
`TX_FREQ` is the frequency of the lowest 4FSK tone in MHz, `FSK4_BAUD` is the desired baud rate (recommended to be 100 baud, though 50 and 300 baud are supported in the decoders), and `FSK4_SPACING` is the spacing between the tones in Hz (should be >=2x the baud rate).

Note that the *acheived* tone spacing depends on the frequency resolution of the radio module. In the case of the SX127x the resolution is 61 Hz, so while I specify a 270 Hz tone spacing, we end up with an acheived tone spacing of 244 Hz.

Once we have our encoded packet, we can transmit as follows:
```c
fsk4_idle(&radio);
delay(1000);
fsk4_preamble(&radio, 8);
fsk4_write(&radio, codedbuffer, coded_len);
```

`fsk4_idle` turns the transmitter on, and configures it to transmit a carrier on the lowest 4FSK tone. `fsk4_preamble` sends a repeated sequence of incrementing 4FSK tones, allowing the decoder to lock-on to the signal. Finally, `fsk4_write` transmits the encoded payload, and turns the transmitter off once done.


## TODO List
* Split out Horus-specific code into separate files, for easier integration into other codebases.