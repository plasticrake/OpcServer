# OpcServer

[![Build Status](https://travis-ci.org/plasticrake/OpcServer.svg?branch=master)](https://travis-ci.org/plasticrake/OpcServer)

Open Pixel Control (OPC) server for Arduino.

Compatible with boards that utilize Arduino style [WiFiServer](https://www.arduino.cc/en/Reference/WiFiServer) / [WiFiClient](https://www.arduino.cc/en/Reference/WiFiClient) APIs.

- **ESP8266**
  - NodeMCU
  - [Adafruit HUZZAH](https://www.adafruit.com/products/2471)
  - ...
- **ATSAMD21**
  - [Arduino Zero](https://www.arduino.cc/en/Main/ArduinoBoardZero) with [WiFi Shield 101](https://www.arduino.cc/en/Main/ArduinoWiFiShield101) **(compiles, not tested)**
  - [Adafruit Feather M0 WiFi - ATSAMD21 + ATWINC1500](https://www.adafruit.com/product/3010) **(compiles, not tested)**
- **Particle** (Formerly Spark)
  - [Photon](https://docs.particle.io/datasheets/photon-datasheet/)
  - [Core](https://docs.particle.io/datasheets/core-datasheet/) **(compiles, not tested)**

If you've run this successfully on other boards let me know!

# Installation

## Arduino IDE
1. In the Arduino IDE, Choose **Sketch** > **Include Library** > **Manage Libraries**
2. Search for **OpcServer**
3. Click **Install**

## [PlatformIO](http://platformio.org/lib/show/350/OpcServer)
1. platformio lib install 350

## Particle
1. Copy files from src/* into your project
