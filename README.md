

![Tiny4FSK_Logo](https://github.com/New-England-Weather-Balloon-Society/Tiny4FSK/assets/66796793/ab7b77fc-2d4b-4a5b-8a1d-bdfebb96c139)
# Tiny4FSK - The Lightweight Horus Binary tracker Built for HABs
**WORK IN PROGRESS** - Please do not rely on this as your only tracking system. Tiny4FSK is still in the R&D phase of development and testing.

**NEW** - New KiCAD Schematic and PCB files uploaded (3/23/24).

**STATE** - PCBs have been ordered and arrived on 3/2/24. They have been assembled and thouroghly tested. Just a few issues have been fixed in the new PCB order, arriving on 3/26/24.

![image](https://github.com/New-England-Weather-Balloon-Society/Tiny4FSK/assets/66796793/5e9ff67a-ee2b-4ce7-aadf-ad8f836a545c)

## What is the Tiny4FSK project?
Tiny4FSK aims to be an ultra-tiny high-altitude tracking system. It aims to run on 1 AAA battery that should last for ~12 hours. It runs on 4FSK (4-frequency shift keying), which means that is separated tones into 4 separate frequencies. (Figure 1.1). Upon this, it uses the [Horus Binary v2](https://github.com/projecthorus/horusdemodlib/wiki/2---Modem-Details#horus-binary-v1-mode-4-fsk) system, which is a relatively modern system popularly used with [RS41ng](https://github.com/mikaelnousiainen/RS41ng).
## What are High-Altitude Balloons?

High-Altitude Ballooning (HAB) offers a formally structured yet thrilling hobby, launching payloads to near-space (30km) for atmospheric research, breathtaking imagery, and scientific experimentation. While demanding meticulous planning, safety adherence, and technical skill in electronics and mechanics, HAB rewards hobbyists with hands-on engineering challenges and atmospheric understanding.
## Parts and Materials
This codebase is meant to work with the Tiny4FSK PCB, also on this GitHub repository. On it are these basic components:

 - **Microcontroller** - [SAMD21G18A](https://www.microchip.com/en-us/product/atsamd21g18). This is a modern Cortex-M0 microcontroller by Microchip. It runs on ARM.
 - **TX Module** - [Si4063](https://www.silabs.com/wireless/proprietary/ezradiopro-sub-ghz-ics/device.si4063?tab=specs). This is a transceiver from Silicon Labs, a derivative of the Si4032 on RS41 radiosondes.
 - **GPS RX Module** - [MAX-M10S](https://www.u-blox.com/en/product/max-m10-series). This is a top-of-the-line GPS module from Ublox.
 - **Environmental Sensor** - [BME280](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/). This is a sensor from Bosch proven to work on HAB flights. Measures temperature, pressure, and humidity.
 - **Boost Converter** - [TPS61200](https://www.ti.com/product/TPS61200). This is a boost converter from Texas Instruments. This is used to convert from 1.5v battery level to 3.3v for the rest of the board.
 - **LDO Step-Down** - [MCP1700](https://www.microchip.com/en-us/product/mcp1700). This is a step down from Microchip.

This list is not at all comprehensive, but this list features the basic components. There are plenty of other passives outlined in the schematic files if you're curious.
## The Codebase
This code is modular and separated into several different files for easy expansion. These files are listed below:

 - **Tiny4FSK.ino** - Main Arduino file with control flow.
 - **config.h** - Configuration file for user parameters.
 - **crc_calc.cpp and crc_calc.h** - CRC16 generator files for parity bits.
 - **horus_l2.cpp and horus_l2.h** - Horus layer 2 file. Golay error correction algorithm.
 - **voltage.cpp and voltage.h** - Voltage detection using ADC values.

## Setting up Arduino IDE
This project is based on the Arduino IDE workflow. Below steps outline steps necessary to install Arduino IDE and configure it for the SAMD microcontroller.

 1. Install [Arduino IDE](https://www.arduino.cc/en/software) from [here](https://www.arduino.cc/en/software).
 2. [Download the Arduino SAMD core](https://docs.arduino.cc/learn/starting-guide/cores/).
 3. Download necessary libraries from library manager:

	 a. ArduinoLowPower

	 b. RadioLib

	 c. Sparkfun GNSS v3

 5. To following needs to be downloaded directly from GitHub:
    
	a. TinyBME280

 4. Open Tiny4FSK.ino by double-clicking it (should open Arduino IDE).

The SAMD goes to sleep to save power. To achieve proper sleep, some edits to the SAMD core are necessary. To find the wiring.c file on your computer, [follow this guide](https:support.arduino.cc/hc/en-us/articles/4415103213714-Find-sketches-libraries-board-cores-and-other-files-on-your-computer).
Once there, comment out or completely delete this line as shown:

 

    // Defining VERY_LOW_POWER breaks Arduino APIs since all pins are considered INPUT at startup
    // However, it really lowers the power consumption by a factor of 20 in low power mode (0.03mA vs 0.6mA)
    #ifndef VERY_LOW_POWER
      // Setup all pins (digital and analog) in INPUT mode (default is nothing)
      for (uint32_t ul = 0 ; ul < NUM_DIGITAL_PINS ; ul++ )
      {
        pinMode( ul, INPUT ) ;
      }
    #endif

Arduino gives you an official warning: "This breaks Arduino APIs since all pins are considered INPUT at startup. However, it really lowers the power consumption by a factor of 20 in low power mode (0.03mA vs 0.6mA)". However, from testing, it doesn't affect anything.

## Code Configuration
User configuration of this tracker is **required**. As this system uses amateur radio, you will need a Technician's level license (US). Configuration file is located in **config.h**. Here are instruction for configuration of these parameters.

`HORUS_ID` - This setting is your Horus ID number. Information on how to get one in next section.

`FSK_FREQ` - This is setting for your preferred TX frequency. If in US, best to keep the same as the PCB is built around 433.200 MHz. Minor modifications in the 70 cm band are fine though.

`FSK_BAUD` - FSK baud rate. No need to change, as most RX station use the baud rate. 

`FSK_SPACING` - FSK spacing in Hz. Once again, most station are set to this value.

`STATUS_LED` - Comment out to disable verbose status LEDs on PCB.

`DEV_MODE` - Comment out for flight mode. Disables Serial and enables deep sleep modes for lower power consumption.

**Everything below these values in the configuration file can go unchanged.** These are pin numbers, and unless you are making your own PCB, leave them be.

`NSS_PIN`- Si4063 SS pin

`RESET_PIN` - Si4063 RST pin

`DIO0_PIN` - Si4063 GPIO0 pin

`DIO1_PIN` - Si4063 GPIO1 pin

`EXTINT` - GPS EXTINT pin for longer packet delays.

`SUCCESS_LED` - Success LED pin.

`ERROR_LED` - Error LED pin.
## How do I get a Horus v2 ID?
If you are going to fly your own payload using Horus Binary, you must get a payload ID allocated for your use. This can be done by  [submitting an issue](https://github.com/projecthorus/horusdemodlib/issues/new/choose)  or a pull request to this repository, or e-mailing VK5QI: vk5qi@rfhead.net

**Do not use the testing (4FSKTEST-V2) payload ID on an actual launch! (ID 256)**
## PCB Configuration
The Tiny4FSK PCB has many configurable operating modes, pins and power sources. This section will outline these parts of the PCB.

The PCB contains two jumpers, JP1 and JP2.

**JP1** - Cut (desolder) to enable SW3 for power.

**JP2** - Control power source (USB and center for USB mode, center and BATT for battery mode).

There are also two switches/buttons, SW1 and SW2.

**SW1** - Reset button for the microcontroller.

**SW2** - Battery power switch. Connects battery output to boost converter input.

There are two antenna pads, AE1 is already soldered (GPS antenna). AE2 is the UHF antenna pad.

**AE2** - 70cm antenna pad. For a guide to cut a proper antenna, refer to the following section.
## Antenna System
This system relies on 70cm (420-450 MHz in the US). There is a pad that supports a quarter-wave monopole, but also provides grounding pads for a more advanced antenna. **Generally, a length of magnet wire ~16cm** should work just fine.
## Contact Me!
To ask questions, kindly donate, or even say hi, feel free to contact me at this email: tiny4fsk@gmail.com. Thanks!
## Cited Works and Sources
https://github.com/mikaelnousiainen/RS41ng - RS41ng Project

https://github.com/projecthorus/horusdemodlib - Horus Binary modulator and decoder repository

https://www.microchip.com/en-us/product/atsamd21g18 - SAMD21 product page

https://www.silabs.com/wireless/proprietary/ezradiopro-sub-ghz-ics/device.si4063 - Si4063 product page

https://www.u-blox.com/en/product/max-m10-series - MAX-M10 product page

https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280 - BME280 product page

https://www.ti.com/product/TPS61200 - TPS61200 product page

https://www.microchip.com/en-us/product/mcp1700 - MCP1700 product page
## Special Thanks
This project would not be possible without the following individuals:
 - Charlie Nicholson KC1SFR for working together on several parts
 - Mark Jessop VK5QI for helping out with every part of this project
 - Mike Hojnowski KD2EAT for helping out with software and hardware issues
 - Bob Phinney K5TEC for providing funding for this project
 - Brett Miwa for schematic and PCB design help
 - Joe KM1P for help with much of the RF part of this project
 - Seth and Jennifer Kendall (Mum and Dad!) for motivating me to work

And these entities:

 - New England Sci-Tech for providing a makerspace to work
 - Sci-Tech Amateur Radio Society for providing expertise
 - New England Weather Balloon Society for providing insights
 - Digikey for being our part sourcing site

## License
Both the software and documentation are under the MIT license, and the hardware is to be licensed soon.
