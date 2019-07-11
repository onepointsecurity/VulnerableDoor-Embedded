# Vulnerable Door
*An IoT Vulnerability Demonstrator created by [onepoint](https://www.groupeonepoint.com/)*

## The project
This software is part of a project by onepoint to demonstrate IoT Vunerabilities.

The goal of the project was to create an alarm system that sends a SMS when the front door is opened without the owner. Configuration and detection is made based on Bluetooth connection.

The project was developed in a start up spirit to be low cost, and fast. It aims to illustrate some classical flaws in IoT products.

All the source code is open source and can be found on the github of onepointsecurity:
- [Object source code](https://github.com/onepointsecurity/VulnerableDoor-Embedded)
- [Companion Android App](https://github.com/onepointsecurity/VulnerableDoor-Android)

An [exploit script](https://github.com/onepointsecurity/VulnerableDoor-Exploit) is also available.

## Vulnerabilities

The project demonstrates the following design flaws:
- Custom made non robust encryption
- Hard coded encryption key in Android application
- Development code active on embedded UART
- Dumpable firmware
- Service account (a.k.a backdoor) hardcoded in the device

# How this works and How to Hack it

See the presentation document in the [exploit repository](https://github.com/onepointsecurity/VulnerableDoor-Exploit)

# Building

## Hardware
The hardware is made from an AVR ATMEGA 328P microcontroler with a SIM800 controler for SMS and a BLE 4.0 module for bluetooth.

The prototype is built using [DVID](https://www.dvid.eu) motherboard.

For detailed instructions see the Wiki in the embedded code repository.

## Software

### Embedded Software

The software needs to be compiled using Arduino IDE with the following parameters:
- Card Type: ATmega328/328p
- Processor: ATmega328p
- Clock: Internal 8 MHz
- Programmer: USBasp

If those parameters cannot be selected, you need to first add the board manager:
- Open File/Preferences
- Enter `https://raw.githubusercontent.com/carlosefr/atmega/master/package_carlosefr_atmega_index.json` as the URL for additional boards
- Validate

Upload the software with an USBasp module plugged on the UART port of the DVID board. Make sure that the selected COM port is correct in Arduino IDE.

### Android Application

The project can be checked out in Android Studio. As an alternative, you can download the APK directly and install it on your smartphone. It needs to support Bluetooth Low Energy standard 4.0 minimum.

### Exploit

Exploit script is developped using Python. Your computer needs to support bluetooth mac spoofing, which is not the case for every supplier.

*Note: It works great with a Rasperry PI 3*

# Limitations

Current versions of Android supports Bluetooth MAC randomization. This is a good thing for user privacy but this prevents the VulnDoor system to work correctly as the identification of the user is based on MAC address.

It should be possible to use Bluetooth Pairing instead of just MAC address, however the (very) low cost BLE module we had does not support pairing with more than 1 client.

Bluetooth randomization is performed sufficiently slowly to permit a demonstration even with a modern Android version so it was choosen to keep it that way. It is just a demonstrator after all ^^
