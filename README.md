# ThingsSentral

By Idlan Zafran Mohd Zaidie

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/yourusername/ThingsSentral)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

An elegant, modular Arduino library for connecting **ESP8266** and **ESP32** devices to the [ThingsSentral.io](http://thingssentral.io) platform. 

## Features

The library is broken down into three core modules:
* **Command Module:** Send and read data in real-time.
* **Vault Module:** Safely store data locally (offline) using LittleFS, and automatically sync it to the cloud when the internet connection is restored.
* **History Module:** Record time-series data with timestamps and upload it in bulk formats using JSON.

## Dependencies

* **LittleFS:** Used internally for the Vault and History modules.
* **ArduinoJson:** Required for parsing and formatting history payloads.

## Installation

### For Arduino IDE
1. Download this repository as a `.zip` file.
2. Open the Arduino IDE.
3. Go to **Sketch** -> **Include Library** -> **Add .ZIP Library...**
4. Select the downloaded `.zip` file.
5. Install the **ArduinoJson** library via the Library Manager (`Tools` -> `Manage Libraries`).

### For PlatformIO
Add the following to your `platformio.ini` file:
```ini
lib_deps =
    ThingsSentral
    bblanchon/ArduinoJson