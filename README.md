# ThingsSentral

**By Idlan Zafran Mohd Zaidie**

[![Version](https://img.shields.io/badge/version-1.1.0-blue.svg)](https://github.com/IdlanZafran/ThingsSentral)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

An enterprise-grade, memory-safe Arduino library for connecting **ESP8266** and **ESP32** devices to the [ThingsSentral.io](http://thingssentral.io) platform. 

Version 1.1.0 brings massive under-the-hood stability improvements, including HTTP Keep-Alive, strict timeout protections, and zero-fragmentation flash memory management.

---

## 🚀 Core Features

This library is architected around three specialized modules, each heavily optimized for microcontroller constraints to prevent memory leaks and socket exhaustion:

* **Command Module:** Built for real-time telemetry. Utilizes **HTTP Keep-Alive** for instant, low-latency connection reuse, bypassing the overhead of constant TCP handshakes.
* **Vault Module:** Bulletproof offline caching using LittleFS. Safely stores data during internet outages and uses a secure swap-file system to automatically sync it back to the cloud without dropping a single data point.
* **History Module:** Records time-series data with precise timestamps. Features **memory-safe appending** (zero RAM overhead) and **automatic JSON chunking** to allow massive bulk uploads without crashing your device.

---

## ⚖️ Module Comparison: Pros & Cons

Choosing the right module depends on whether your project prioritizes real-time control, data retention, or power efficiency. 

| Module | Best Use Case | Pros | Cons |
| :--- | :--- | :--- | :--- |
| **Command** | Smart home devices, live dashboards, remote relay control. | **+ Two-Way Sync:** The *only* module that can both send telemetry and receive remote commands.<br>**+ Instantaneous:** Data appears on your dashboard instantly.<br>**+ Low Latency:** Keep-Alive reuse makes HTTP requests incredibly fast. | **- No Fallback:** If the WiFi drops, the data point is lost forever.<br>**- Network Dependent:** Slow networks can block your main loop if not carefully timed. |
| **Vault** | Industrial monitors, mobile asset trackers, spotty WiFi zones. | **+ Zero Data Loss:** Guarantees data delivery even after hours of network downtime.<br>**+ Auto-Recovery:** Seamlessly syncs cached data the moment the connection returns.<br>**+ Flash Friendly:** Can be programmed to attempt live sends first, saving flash write cycles. | **- Outbound Only:** Cannot receive instructions from the server.<br>**- Delayed Data:** Dashboard metrics stall until the device comes back online.<br>**- Flash Wear:** Heavy offline usage will eventually wear out the ESP's flash memory. |
| **History** | Battery-powered devices, deep-sleep loggers, bulk data analytics. | **+ Extreme Efficiency:** Perfect for devices that wake up, read a sensor, log it, and go back to sleep.<br>**+ Network Saving:** Reduces WiFi radio uptime by batching hundreds of readings into a single upload.<br>**+ Crash-Proof:** Chunking logic prevents heap fragmentation during massive HTTP payloads. | **- Outbound Only:** Cannot receive instructions from the server.<br>**- Requires NTP:** Timestamps will be completely wrong if the device fails to sync with an NTP server on boot.<br>**- Not Real-Time:** Data is only viewable after a bulk upload is triggered. |

---

## 📦 Dependencies

To ensure maximum stability, this library relies on the following core components:
* **LittleFS:** Used internally for the Vault and History modules to handle file storage. *(Built into the ESP core)*.
* **ArduinoJson:** Required for parsing incoming commands and formatting history payloads. **(v6.x or v7.x supported)**.

---

## 🛠️ Installation

### For Arduino IDE
1. Open the Arduino IDE.
2. Go to **Sketch** -> **Include Library** -> **Manage Libraries...**
3. In the search bar, type **ThingsSentral** by Idlan Zafran Mohd Zaidie.
4. Click **Install**.
5. Repeat the search for **ArduinoJson** and install it if you haven't already.

### For PlatformIO
Add the following to your `platformio.ini` file under your environment configuration:
```ini
lib_deps =
    [https://github.com/IdlanZafran/ThingsSentral.git](https://github.com/IdlanZafran/ThingsSentral.git)
    bblanchon/ArduinoJson
```