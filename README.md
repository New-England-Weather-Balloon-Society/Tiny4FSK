![Tiny4FSK_Logo](https://cloud-nhw66iv7j-hack-club-bot.vercel.app/0logo.svg)
# Tiny4FSK - The Lightweight Horus Binary Tracker Built for HABs
**WORK IN PROGRESS** - Please do not rely on this as your only tracking system. Tiny4FSK is still in the R&D phase of development and testing.

**NEW** - Incorporated new Rev. 4 PCB files and code (12/21/24).

**STATE** - Rev. 4 is fully working! Currently testing.

## What is the Tiny4FSK project?
Tiny4FSK aims to be an ultra-tiny high-altitude tracking system. It runs on 1 AA battery that lasts for 10-17 hours (a few seconds between position updates, can run longer if there's a longer delay). It runs on 4FSK (4-frequency shift keying), which means that it separates tones into 4 separate frequencies. Upon this, it uses the [Horus Binary v2](https://github.com/projecthorus/horusdemodlib/wiki/2---Modem-Details#horus-binary-v1-mode-4-fsk) system, which is a relatively modern system popularly used with [RS41ng](https://github.com/mikaelnousiainen/RS41ng).

![20240826_110047](https://cloud-ivkikeghh-hack-club-bot.vercel.app/0img_20241231_123601.jpg)

## What are High-Altitude Balloons?
High-Altitude Ballooning (HAB) offers a formally structured yet thrilling hobby, launching payloads to near-space (30km) for atmospheric research, breathtaking imagery, and scientific experimentation. While demanding meticulous planning, safety adherence, and technical skill in electronics and mechanics, HAB rewards hobbyists with hands-on engineering challenges and atmospheric understanding.
## Parts and Materials
This codebase is meant to work with the Tiny4FSK PCB, also available on this GitHub repository. Key components include:

 - **Microcontroller** - [SAMD21G18A](https://www.microchip.com/en-us/product/atsamd21g18). A modern Cortex-M0+ microcontroller by Microchip.
 - **TX Module** - [Si4063](https://www.silabs.com/wireless/proprietary/ezradiopro-sub-ghz-ics/device.si4063?tab=specs). A transceiver from Silicon Labs, similar to the Si4032 on RS41 radiosondes.
 - **GPS RX Module** - [ATGM336H](https://jlcpcb.com/partdetail/Zhongkewei-ATGM336H5N31/C90770). A cheap, high-performance GPS module from ZHONGKEWEI.
 - **Environmental Sensor** - [BME280](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/). A proven sensor from Bosch for measuring temperature, pressure, and humidity.
 - **Boost Converter** - [TPS61200](https://www.ti.com/product/TPS61200). Boosts 1.5V battery level to 3.3V for the rest of the board from Texas Instruments.
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
 - **4fsk_mod.cpp and 4fsk_mod.h** - 4FSK modulation functions.
 - **delay_timer.cpp and delay_timer.h** - Low-level delay functions based on timers.
 - **utils.cpp and utils.h** - A collection of utility functions.


# Step by Step Setup Guide
## Setting up the PCB
1. **Carefully** snap off the USB breakout board from the board.
2. Solder on the female 1x4 header row onto the underside of the USB breakout. Solder on the 1x4 male header row to the corresponding adjecent pins (labeled VSS, GND, D-, and D+).

![Solder diagram](https://cloud-15azhb7wk-hack-club-bot.vercel.app/0usb-solder.jpg)
3. To program, insert the USB breakout such that it is self-contained.

![USB Connection Diagram](https://cloud-fy2mwui0k-hack-club-bot.vercel.app/0usb-diagram.jpg)

## Antenna System
This system relies on the 70cm amateur radio band (420-450 MHz in the US). A braided copper *monopole* is included cut to the perfect length for this frequency. The included antenna is a quarter-wave monopole, meaning that the antenna is just a quarter of the wavelength of the signal, and yet it maintains a good SWR (quantification of how much signal is reflected back into the transmitter). There is a pad that supports a quarter-wave monopole, but also provides grounding pads for a more advanced antenna.
<details>
<summary>Why does this work?</summary>
TODO
</details>
<br>

Solder the copper wire onto the center pin of the antenna connector (as depicted below).

<!-- ![Antenna Connection](https://cloud-nscrzqsdm-hack-club-bot.vercel.app/0antenna_diagram.png) -->

## PCB Configuration
The Tiny4FSK PCB has many configurable operating modes, pins and power sources. This section will outline these parts of the PCB.

The PCB contains one jumper, JP1.

- **JP1** - Cut (desolder) to enable SW1 for power. During flight, the mechanical shock from the landing can cause the power switch to flip, so solder this jumper to disable this risk.

There are also two switches/buttons, SW1 and SW2.

- **SW1** - Battery power switch. Connects battery output to boost converter input.
- **SW2** - Reset button for the microcontroller.

There are two antenna pads, AE1 and AE2.
- **AE1** - L1 GPS band antenna
- **AE2** - 70cm antenna pad. For a guide to cut a proper antenna, refer to the following section.

## Setting up the Code
Now that you've got all the hardware set up, time for programming!

## Setting up Arduino IDE
This project is based on the Arduino IDE workflow. Below steps outline steps necessary to install Arduino IDE and configure it for the SAMD microcontroller.

 1. Install [Arduino IDE](https://www.arduino.cc/en/software) from [here](https://www.arduino.cc/en/software).
 2. [Download the Arduino SAMD core](https://docs.arduino.cc/learn/starting-guide/cores/).
 3. Download necessary libraries from library manager:
    * ArduinoLowPower
    * TinyGPSPlus
    * Scheduler
 5. To following needs to be downloaded directly from GitHub:
    * [TinyBME280](https://github.com/maxsrobotics/tiny-bme280/)

**Optional** - The SAMD goes to sleep to save power. To achieve proper sleep, some edits to the SAMD core are necessary. To locate the wiring.c file on your computer, [follow this guide](https:support.arduino.cc/hc/en-us/articles/4415103213714-Find-sketches-libraries-board-cores-and-other-files-on-your-computer).
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
User configuration of this tracker is **required**. As this system uses amateur radio, you will need at least a Technician's level license (US). Configuration file is located in **config.h**. Open this file in Arduino IDE. Here are the parameters may need to be changed.

- `HORUS_ID` - This setting is your Horus ID number. Information on how to get one in next section.
- `CALLSIGN` - Amateur radio callsign. This is required to stay legal!
- `CALLSIGN_WPM` - Speed to send the callsign, in morse code.
- `CALLSIGN_INTERVAL` - Interval to send the morse code callsign. Maximum interval in the US is 10 minutes.
- `FSK_FREQ` - This is setting for your preferred TX frequency. The filter is optimized for 70cm radio band.
- `STATUS_LED` - Comment out to disable verbose status LEDs on PCB.
- `DEV_MODE` - Comment out for flight mode. Disables Serial and enables deep sleep modes for lower power consumption.
- `PACKET_INTERVAL` - Interval between 4FSK packets. The smaller the interval, the lower the battery life is.
- `OUTPUT_POWER` - 0-127. This is the output power of the radio module (suggested to keep at maximum).
- `FLAG_BAD_PACKET` - If the latest GPS values are bad, send out all zeroes (for time, position, speed, and altitude)(suggested).

**Everything below the above values in the configuration file can go unchanged.** These are various other settings that may be useful for development.
- `FSK_BAUD` - FSK baud rate. No need to change, as most RX station use 100 baud. 
- `FSK_SPACING` - FSK spacing in Hz. Most station are set to this value (270hz).
- `NSS_PIN`- Si4063 SS pin
- `RESET_PIN` - Si4063 RST pin
- `DIO0_PIN` - Si4063 GPIO0 pin
- `DIO1_PIN` - Si4063 GPIO1 pin
- `EXTINT` - GPS EXTINT pin for longer packet delays.
- `SUCCESS_LED` - Success LED pin.
- `ERROR_LED` - Error LED pin.
- `VOLTMETER_PIN` - Pin for the onboard voltage divider for voltage sensing.

<details>
<summary>How do I change these values?</summary>
If there is a prexisting number or value next to the name of the setting name, you can replace that value with the desired value (e.g. replace the "N0CALL" with your callsign in double quotes, "W0MXX"). If there is no value next to the name, you need to comment out the setting to disable that functionality, or uncomment to enable that functionality (comments are defined by adding // at the start of the line).
</details>

### How do I get a Horus v2 ID?
If you are going to fly your own payload using Horus Binary, you must get a payload ID allocated for your use. This can be done by  [submitting an issue](https://github.com/projecthorus/horusdemodlib/issues/new/choose) or a pull request to that repository, or e-mailing VK5QI: vk5qi@rfhead.net

**Do not use the testing (4FSKTEST-V2) payload ID on an actual launch! (ID 256)**

## Upload the Code!
Once code configuration is complete, you may plug in a standard data USB-C cable into the breakout board, select the port in Arduino IDE, and select the upload button (marked by an arrow at the top).

## Testing
Once the code is uploaded, you'll see the green LED light start to glow. The behavior of that LED is outlined [here](#led-default-behavior). Now you'll need to set up a receive station on either a [laptop or computer with an SDR](https://github.com/projecthorus/horusdemodlib/wiki/1.1-Horus-GUI-Reception-Guide-(Windows-Linux-OSX)), or a [Raspberry Pi board connected to an SDR](https://github.com/projecthorus/horusdemodlib/wiki/1.2--Raspberry-Pi-'Headless'-RX-Guide).

You should start decoding packets transmitted from the board with your configuration parameters. It may take up to 5 minutes to gain a GPS fix, depending on your location.

## LED Default Behavior
| Green LED Behavior           | Meaning                     |
|------------------------------|-----------------------------|
| Solid                        | Initialization complete     |
| Blinking (1s on, 1s off)     | GPS detected and configured |
| Blinking (0.5s on, 0.5s off) | Transmission complete       |
| Off                          | Deep sleep mode or error    |


## PCBWay PCBs
The new Revision 4 PCBs have been fabricated and assembled through PCBWay. Their high-quality fabrication and assembly services is truly commendable. I appreciated the ability to choose from many different component suppliers to select the exact components I needed. Additionally, their customer service is incredibly responsive and helpful, and quickly notified me of design issues. I highly recommend PCBWay for any of your PCB prototyping & assembly needs. Thank you, PCBWay, for graciously sponsoring this project!

## Contact Me!
To ask questions, kindly donate, or even say hi, feel free to contact me at this email: tiny4fsk@gmail.com. Thanks!

## Cited Works and Sources
- [RS41ng Project](https://github.com/mikaelnousiainen/RS41ng)
- [Horus Binary modulator and decoder repository](https://github.com/projecthorus/horusdemodlib)
- [SAMD21 product page](https://www.microchip.com/en-us/product/atsamd21g18)
- [Si4063 product page](https://www.silabs.com/wireless/proprietary/ezradiopro-sub-ghz-ics/device.si4063)
- [ATGM336H product page](https://www.u-blox.com/en/product/max-m10-series)
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
 - PCBWay for PCB prototyping and sponsorship!
 - Bagel Fund for sponsorship!
 - Hack Club for support and sponsorship!
 - New England Sci-Tech for providing a makerspace.
 - Sci-Tech Amateur Radio Society for expertise.
 - New England Weather Balloon Society for insights.
 - Digikey for part sourcing.

## License
Software and Documentation:
- [GNU GPL v3 license](https://choosealicense.com/licenses/gpl-3.0/)

Hardware License:
- [OSHW (US002611)](https://certification.oshwa.org/us002611.html) and [CERN Open Hardware Licence Version 2 - Weakly Reciprocal](https://choosealicense.com/licenses/cern-ohl-w-2.0/)

## Pictures
![20240826_110047](https://cloud-ivkikeghh-hack-club-bot.vercel.app/0img_20241231_123601.jpg)
![](https://cloud-as6j73c1n-hack-club-bot.vercel.app/0img_6985.jpg)
![](https://cloud-as6j73c1n-hack-club-bot.vercel.app/1img_20241231_123232.jpg)
![](https://cloud-as6j73c1n-hack-club-bot.vercel.app/2img_6986.jpghttps://cloud-as6j73c1n-hack-club-bot.vercel.app/0img_6985.jpg)
![](https://cloud-as6j73c1n-hack-club-bot.vercel.app/3img_6981.jpg)
![](https://cloud-as6j73c1n-hack-club-bot.vercel.app/0img_6985.jpg)
![](https://cloud-as6j73c1n-hack-club-bot.vercel.app/4img_6980.jpg)