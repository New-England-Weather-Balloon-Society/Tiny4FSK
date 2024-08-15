![Tiny4FSK_Logo](https://github.com/New-England-Weather-Balloon-Society/Tiny4FSK/assets/66796793/ab7b77fc-2d4b-4a5b-8a1d-bdfebb96c139)
# Tiny4FSK - The Lightweight Horus Binary Tracker Built for HABs
**WORK IN PROGRESS** - Please do not rely on this as your only tracking system. Tiny4FSK is still in the R&D phase of development and testing.

**NEW** - Significant repository structure overhaul (6/27/24).

**STATE** - Fully working tracker board! In the phase of testing and minor tweaks.

## What is the Tiny4FSK project?
Tiny4FSK aims to be an ultra-tiny high-altitude tracking system. It runs on 1 AA battery that lasts for 10-17 hours (a few seconds between position updates, can run longer if there's a longer delay). It runs on 4FSK (4-frequency shift keying), which means that it separates tones into 4 separate frequencies. Upon this, it uses the [Horus Binary v2](https://github.com/projecthorus/horusdemodlib/wiki/2---Modem-Details#horus-binary-v1-mode-4-fsk) system, which is a relatively modern system popularly used with [RS41ng](https://github.com/mikaelnousiainen/RS41ng).
![IMG_0959](https://github.com/New-England-Weather-Balloon-Society/Tiny4FSK/assets/66796793/af6c3054-d633-4bb9-8cf3-433f28d4ad0c)


## What are High-Altitude Balloons?

High-Altitude Ballooning (HAB) offers a formally structured yet thrilling hobby, launching payloads to near-space (30km) for atmospheric research, breathtaking imagery, and scientific experimentation. While demanding meticulous planning, safety adherence, and technical skill in electronics and mechanics, HAB rewards hobbyists with hands-on engineering challenges and atmospheric understanding.
## Parts and Materials
This codebase is meant to work with the Tiny4FSK PCB, also available on this GitHub repository. Key components include:

 - **Microcontroller** - [SAMD21G18A](https://www.microchip.com/en-us/product/atsamd21g18). A modern Cortex-M0+ microcontroller by Microchip.
 - **TX Module** - [Si4063](https://www.silabs.com/wireless/proprietary/ezradiopro-sub-ghz-ics/device.si4063?tab=specs). A transceiver from Silicon Labs, similar to the Si4032 on RS41 radiosondes.
 - **GPS RX Module** - [MAX-M10S](https://www.u-blox.com/en/product/max-m10-series). A high-performance GPS module from Ublox.
 - **Environmental Sensor** - [BME280](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/). A proven sensor from Bosch for measuring temperature, pressure, and humidity.
 - **Boost Converter** - [TPS61200](https://www.ti.com/product/TPS61200). Boosts 1.5V battery level to 3.3V for the rest of the board.
 - **LDO Step-Down** - [MCP1700](https://www.microchip.com/en-us/product/mcp1700). A step-down converter from Microchip.

This list is not exhaustive. Additional components and passives are detailed in the schematic files.
## The Codebase
This code is modular and separated into several different files for easy expansion. These files are listed below:

 - **Tiny4FSK.ino** - Main Arduino file with control flow.
 - **config.h** - Configuration file for user parameters.
 - **crc_calc.cpp and crc_calc.h** - CRC16 generator files for parity bits.
 - **horus_l2.cpp and horus_l2.h** - Horus layer 2 file, Golay error correction algorithm.
 - **voltage.cpp and voltage.h** - Voltage detection using ADC values.
 - **si4063.cpp and si4063.h** - Si4063 driver files for radio transmission.
 - **fsk4_mod.cpp and fsk4_mod.h** - 4FSK modulation functions.
 - **delay_timer.cpp and delay_timer.h** - Low-level delay functions based on timers.
 - **i2c_scan.cpp and i2c_scan.h** - I2C scanning for debugging utility.

## Setting up Arduino IDE
This project is based on the Arduino IDE workflow. Below steps outline steps necessary to install Arduino IDE and configure it for the SAMD microcontroller.

 1. Install [Arduino IDE](https://www.arduino.cc/en/software) from [here](https://www.arduino.cc/en/software).
 2. [Download the Arduino SAMD core](https://docs.arduino.cc/learn/starting-guide/cores/).
 3. Download necessary libraries from library manager:
	a. ArduinoLowPower
	b. Sparkfun GNSS v3
 5. To following needs to be downloaded directly from GitHub:
	a. [TinyBME280](https://github.com/technoblogy/tiny-bme280/)
 4. Open Tiny4FSK.ino by double-clicking it (should open Arduino IDE).

The SAMD goes to sleep to save power. To achieve proper sleep, some edits to the SAMD core are necessary. To locate the wiring.c file on your computer, [follow this guide](https:support.arduino.cc/hc/en-us/articles/4415103213714-Find-sketches-libraries-board-cores-and-other-files-on-your-computer).
Once there, comment out or completely delete this line as shown:
```cpp
    // Defining VERY_LOW_POWER breaks Arduino APIs since all pins are considered INPUT at startup
    // However, it really lowers the power consumption by a factor of 20 in low power mode (0.03mA vs 0.6mA)
    #ifndef VERY_LOW_POWER
      // Setup all pins (digital and analog) in INPUT mode (default is nothing)
      for (uint32_t ul = 0 ; ul < NUM_DIGITAL_PINS ; ul++ )
      {
        pinMode( ul, INPUT ) ;
      }
    #endif
```
Arduino gives you an official warning: "This breaks Arduino APIs since all pins are considered INPUT at startup. However, it really lowers the power consumption by a factor of 20 in low power mode (0.03mA vs 0.6mA)". Testing has shown no adverse effects.

## Code Configuration
User configuration of this tracker is **required**. As this system uses amateur radio, you will need at least a Technician's level license (US). Configuration file is located in **config.h**. Here are the parameters that need to be changed.

- `HORUS_ID` - This setting is your Horus ID number. Information on how to get one in next section.
- `FSK_FREQ` - This is setting for your preferred TX frequency. If in US, best to keep the same as the PCB is built around 433.200 MHz. Minor modifications in the 70 cm band are fine though.
- `FSK_BAUD` - FSK baud rate. No need to change, as most RX station use the baud rate. 
- `FSK_SPACING` - FSK spacing in Hz. Once again, most station are set to this value.
- `STATUS_LED` - Comment out to disable verbose status LEDs on PCB.
- `DEV_MODE` - Comment out for flight mode. Disables Serial and enables deep sleep modes for lower power consumption.

**Everything below the above values in the configuration file can go unchanged.** These are pin numbers, and unless you are making your own PCB, leave them be.

- `NSS_PIN`- Si4063 SS pin
- `RESET_PIN` - Si4063 RST pin
- `DIO0_PIN` - Si4063 GPIO0 pin
- `DIO1_PIN` - Si4063 GPIO1 pin
- `EXTINT` - GPS EXTINT pin for longer packet delays.
- `SUCCESS_LED` - Success LED pin.
- `ERROR_LED` - Error LED pin.

## How do I get a Horus v2 ID?
If you are going to fly your own payload using Horus Binary, you must get a payload ID allocated for your use. This can be done by  [submitting an issue](https://github.com/projecthorus/horusdemodlib/issues/new/choose)  or a pull request to this repository, or e-mailing VK5QI: vk5qi@rfhead.net

**Do not use the testing (4FSKTEST-V2) payload ID on an actual launch! (ID 256)**
## PCB Configuration
The Tiny4FSK PCB has many configurable operating modes, pins and power sources. This section will outline these parts of the PCB.

The PCB contains two jumpers, JP1 and JP2.

- **JP1** - Cut (desolder) to enable SW3 for power.
- **JP2** - Control power source (USB and center for USB mode, center and BATT for battery mode).

There are also two switches/buttons, SW1 and SW2.

- **SW1** - Reset button for the microcontroller.
- **SW2** - Battery power switch. Connects battery output to boost converter input.

There are two antenna pads, AE1 and AE2.
- **AE1** - L1 GPS band antenna
- **AE2** - 70cm antenna pad. For a guide to cut a proper antenna, refer to the following section.
## Antenna System
This system relies on 70cm (420-450 MHz in the US). There is a pad that supports a quarter-wave monopole, but also provides grounding pads for a more advanced antenna. **Generally, a length of magnet wire ~16cm** should work just fine.
## Contact Me!
To ask questions, kindly donate, or even say hi, feel free to contact me at this email: tiny4fsk@gmail.com. Thanks!
## Cited Works and Sources
- [RS41ng Project](https://github.com/mikaelnousiainen/RS41ng)
- [Horus Binary modulator and decoder repository](https://github.com/projecthorus/horusdemodlib)
- [SAMD21 product page](https://www.microchip.com/en-us/product/atsamd21g18)
- [Si4063 product page](https://www.silabs.com/wireless/proprietary/ezradiopro-sub-ghz-ics/device.si4063)
- [MAX-M10 product page](https://www.u-blox.com/en/product/max-m10-series)
- [BME280 product page](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280)
- [TPS61200 product page](https://www.ti.com/product/TPS61200)
- [MCP1700 product page](https://www.microchip.com/en-us/product/mcp1700)

## Special Thanks
This project would not be possible without the following individuals:
 - Charlie Nicholson KC1SFR for collaboration.
 - Mark Jessop VK5QI for extensive support.
 - Mike Hojnowski KD2EAT for helping out with software and hardware issues.
 - Bob Phinney K5TEC for providing funding for this project.
 - Brett Miwa for schematic and PCB design help.
 - Joe KM1P for RF help.
 - My parents for motivation.

And these entities:
 - New England Sci-Tech for providing a makerspace.
 - Sci-Tech Amateur Radio Society for expertise.
 - New England Weather Balloon Society for insights.
 - Digikey for part sourcing.

## License
Software and Documentation:
- [GNU GPL v3 license](https://choosealicense.com/licenses/gpl-3.0/)
Hardware License
- [OSHW (US002611)](https://certification.oshwa.org/us002611.html) and [CERN Open Hardware Licence Version 2 - Weakly Reciprocal](https://choosealicense.com/licenses/cern-ohl-w-2.0/)

## Pictures
![IMG_0972](https://github.com/New-England-Weather-Balloon-Society/Tiny4FSK/assets/66796793/eb7dd175-a9ce-46be-b1bb-37f851c07086)
![IMG_0967](https://github.com/New-England-Weather-Balloon-Society/Tiny4FSK/assets/66796793/8c1a57e0-a005-43ad-9a12-7db82b8bdd78)
![IMG_0961](https://github.com/New-England-Weather-Balloon-Society/Tiny4FSK/assets/66796793/18126cac-9f5c-447b-8d6e-ad13160e6982)
![IMG_0959](https://github.com/New-England-Weather-Balloon-Society/Tiny4FSK/assets/66796793/fce14f4f-bd4a-4b16-a40b-19e351b87616)
