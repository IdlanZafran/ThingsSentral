/*
 * Example: History Module (Time-Series Batch Upload)
 * Author: Idlan Zafran Mohd Zaidie
 * * Description:
 * This sketch demonstrates the ThingsSentral History module for bulk data handling. 
 * It syncs the device with an NTP time server, stamps local sensor readings with a 
 * Unix timestamp, and saves them to memory. Once a specific number of records is 
 * collected, it safely chunks and uploads the bulk JSON payload to the server.
 */

#include <Arduino.h>
#include <WiFi.h> // Use <ESP8266WiFi.h> if using ESP8266
#include "ThingsSentral.h"

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

String myUserID = "00953"; 

// Non-blocking timer and optimized counter
unsigned long lastSampleTime = 0;
const unsigned long sampleInterval = 2000; // 2 seconds
int localRecordCount = 0; 

void setup() {
    Serial.begin(115200);
    
    // 1. Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");

    // 2. Initialize ThingsSentral
    TS.begin(myUserID); 

    // 3. Sync Internet Time (Crucial for History timestamps)
    TS.syncTime("MYT-8"); 
    
    // Initialize our counter with whatever is already saved offline
    localRecordCount = TS.History.count();
}

void loop() {
    if (millis() - lastSampleTime >= sampleInterval) {
        lastSampleTime = millis();

        String tempSensorID = "0095312010101"; 
        String humSensorID  = "0095312010102"; 
        
        // Simulated sensor readings
        float currentTemp = random(200, 300) / 10.0; 
        float currentHum  = random(400, 800) / 10.0; 
        
        // 1. Stamp both data points (attaches Unix timestamp automatically)
        TS.History.stamp(tempSensorID, currentTemp);
        TS.History.stamp(humSensorID, currentHum);
        
        // Increment our RAM counter by 2 (since we added 2 records)
        localRecordCount += 2;
        Serial.println("Records ready for upload: " + String(localRecordCount));

        // 2. If we have collected 10 records, upload them using our new auto-chunking logic
        if (localRecordCount >= 10) {
            if (TS.isOnline()) {
                Serial.println("Uploading batch history to server...");
                
                if (TS.History.upload()) {
                    Serial.println("Batch uploaded successfully! Buffer cleared.");
                    localRecordCount = 0; // Reset counter only on success
                } else {
                    Serial.println("Upload failed. Will retry next cycle.");
                }
            } else {
                Serial.println("Cannot upload batch: No internet.");
            }
        }
    }
}