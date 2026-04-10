# ThingsSentral

By Idlan Zafran Mohd Zaidie

[![Version](https://img.shields.io/badge/version-1.1.0-blue.svg)](https://github.com/IdlanZafran/ThingsSentral)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

An enterprise-grade, memory-safe Arduino library for connecting **ESP8266** and **ESP32** devices to the [ThingsSentral.io](http://thingssentral.io) platform. 

Version 1.0.0 brings massive under-the-hood stability improvements, including HTTP Keep-Alive, strict timeout protections, and zero-fragmentation flash memory management.

## Features

The library is broken down into three core modules, each heavily optimized for microcontroller constraints:
* **Command Module:** Send and read data in real-time. Utilizes **HTTP Keep-Alive** for instant, low-latency connection reuse, bypassing the overhead of constant TCP handshakes.
* **Vault Module:** Bulletproof offline caching using LittleFS. Safely stores data during internet outages and uses a secure swap-file system to automatically sync it back to the cloud without dropping a single data point.
* **History Module:** Record time-series data with timestamps. Features **memory-safe appending** (zero RAM overhead) and **automatic JSON chunking** to allow massive bulk uploads without crashing your device.

## Dependencies

* **LittleFS:** Used internally for the Vault and History modules.
* **ArduinoJson:** Required for parsing and formatting history payloads (v6.x or v7.x supported).

## Installation

### For Arduino IDE
1. Open the Arduino IDE.
2. Go to **Sketch** -> **Include Library** -> **Manage Libraries...**
3. In the search bar, type **ThingsSentral**.
4. Click **Install**.
5. Repeat the search for **ArduinoJson** and install it if you haven't already.

### For PlatformIO
Add the following to your `platformio.ini` file:
```ini
lib_deps =
    https://github.com/IdlanZafran/ThingsSentral.git
    bblanchon/ArduinoJson