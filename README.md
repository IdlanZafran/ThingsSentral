# ThingsSentral

**By Idlan Zafran Mohd Zaidie**

[![Version](https://img.shields.io/badge/version-1.1.0-blue.svg)](https://github.com/IdlanZafran/ThingsSentral)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

An enterprise-grade, memory-safe Arduino library for connecting **ESP8266** and **ESP32** devices to the [ThingsSentral.io](http://thingssentral.io) platform. 

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
| **Vault** | Industrial monitors, mobile asset trackers, spotty WiFi zones. | **+ Zero Data Loss:** Guarantees data delivery even after hours of network downtime.<br>**+ Auto-Recovery:** Seamlessly syncs cached data the moment the connection returns.<br>**+ Flash Friendly:** Can attempt live sends first, saving flash write cycles. | **- Outbound Only:** Cannot receive instructions from the server.<br>**- Delayed Data:** Dashboard metrics stall until the device comes back online.<br>**- Flash Wear:** Heavy offline usage will eventually wear out the ESP's flash memory. |
| **History** | Battery-powered devices, deep-sleep loggers, bulk data analytics. | **+ Extreme Efficiency:** Perfect for devices that wake up, read a sensor, log it, and sleep.<br>**+ Network Saving:** Reduces WiFi radio uptime by batching hundreds of readings.<br>**+ Crash-Proof:** Chunking logic prevents heap fragmentation during massive HTTP payloads. | **- Outbound Only:** Cannot receive instructions from the server.<br>**- Requires NTP:** Timestamps will be wrong if the device fails to sync on boot.<br>**- Not Real-Time:** Data is only viewable after a bulk upload is triggered. |

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
3. In the search bar, type **ThingsSentral**.
4. Click **Install**.
5. Repeat the search for **ArduinoJson** and install it if you haven't already.

### For PlatformIO
Add the following to your `platformio.ini` file under your environment configuration:
```ini
lib_deps =
    https://github.com/IdlanZafran/ThingsSentral.git
    bblanchon/ArduinoJson
```

---

## 📚 Quick Start & Examples

*Note: The following examples utilize the [RapidBootWiFi](https://github.com/IdlanZafran/RapidBootWiFi) library to handle headless WiFi provisioning and dynamic User ID injection.*

### 1. Command Module (Live Send & Receive)
Perfect for Smart Plugs or live dashboards requiring instant telemetry and remote control.

```cpp
#include <Arduino.h>
#include <RapidBootWiFi.h>
#include <ThingsSentral.h>

unsigned long lastSendTime = 0;

void setup() {
    Serial.begin(115200);
    
    // Headless Setup
    myWiFi.setAPName("TS_Command_Node");
    myWiFi.addParameter("uid", "ThingsSentral User ID", "00953", 15);
    myWiFi.begin();

    // Initialize ThingsSentral with the User ID saved from the portal
    TS.begin(String(myWiFi.getParameterValue("uid"))); 
}

void loop() {
    myWiFi.loop(); // Keep WiFi alive

    // Send and Read every 10 seconds
    if (millis() - lastSendTime >= 10000) {
        lastSendTime = millis(); 

        if (TS.isOnline()) {
            // SEND Live Data
            TS.Command.send("0095312010101", "24.5"); // Temp
            TS.Command.send("0095312010102", "60.2"); // Humidity

            // READ Live Commands (e.g., Dashboard Switches)
            ReadResult relayState = TS.Command.read("0095312010103");
            Serial.println("Relay State: " + relayState.value); 
        }
    }
}
```

### 2. Vault Module (Dynamic Flash-Saving Routing)
Perfect for mobile trackers or monitors in spotty WiFi zones. Tries to send live; falls back to offline memory if the network drops.

```cpp
#include <Arduino.h>
#include <RapidBootWiFi.h>
#include <ThingsSentral.h>

unsigned long lastReadTime = 0;

void setup() {
    Serial.begin(115200);
    
    myWiFi.setAPName("TS_Vault_Node");
    myWiFi.addParameter("uid", "ThingsSentral User ID", "00953", 15);
    myWiFi.begin();

    TS.begin(String(myWiFi.getParameterValue("uid"))); 
}

void loop() {
    myWiFi.loop();

    if (millis() - lastReadTime >= 5000) {
        lastReadTime = millis();

        String tempID = "0095312010101"; 
        String currentTemp = String(random(200, 300) / 10.0); 

        // 1. Try to send live data directly to the server first
        bool tempSent = false;
        if (TS.isOnline()) tempSent = TS.Command.send(tempID, currentTemp);

        // 2. Fallback routing
        if (!tempSent) {
            TS.Vault.add(tempID, currentTemp);
            Serial.println("Network offline. Saved to Vault.");
        }

        // 3. Housekeeping: Sync any old data trapped in the Vault
        if (TS.isOnline()) TS.Vault.sync(); 
    }
}
```

### 3. History Module (Time-Series Batch Upload)
Perfect for battery-powered or deep-sleep devices. Collects data locally with timestamps and uploads in bulk.

```cpp
#include <Arduino.h>
#include <RapidBootWiFi.h>
#include <ThingsSentral.h>

unsigned long lastSampleTime = 0;
int localRecordCount = 0; 

void setup() {
    Serial.begin(115200);
    
    myWiFi.setAPName("TS_History_Node");
    myWiFi.addParameter("uid", "ThingsSentral User ID", "00953", 15);
    myWiFi.begin();

    TS.begin(String(myWiFi.getParameterValue("uid"))); 

    // Sync Internet Time (Crucial for History timestamps)
    if (TS.isOnline()) TS.syncTime("MYT-8"); 
    
    localRecordCount = TS.History.count();
}

void loop() {
    myWiFi.loop();

    // Sample every 2 seconds
    if (millis() - lastSampleTime >= 2000) {
        lastSampleTime = millis();

        // 1. Stamp data points (attaches Unix timestamp automatically)
        TS.History.stamp("0095312010101", 24.5);
        localRecordCount++;

        // 2. Upload batch if we have 10 or more records
        if (localRecordCount >= 10 && TS.isOnline()) {
            Serial.println("Uploading batch history...");
            if (TS.History.upload()) {
                Serial.println("Batch uploaded successfully!");
                localRecordCount = 0; 
            }
        }
    }
}
```