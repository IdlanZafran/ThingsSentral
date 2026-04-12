/*
 * Example: History Module (Time-Series Batch Upload)
 * Author: Idlan Zafran Mohd Zaidie
 * * Description:
 * Syncs the device with an NTP time server, stamps local sensor readings with a 
 * Unix timestamp, and saves them. Once 10 records are collected, it chunks 
 * and uploads the bulk JSON payload to the server.
 */

#include <Arduino.h>
#include <RapidBootWiFi.h>
#include "ThingsSentral.h"

char defaultUserID[16] = "00953"; 

unsigned long lastSampleTime = 0;
const unsigned long sampleInterval = 2000; // 2 seconds
int localRecordCount = 0; 

void setup() {
    Serial.begin(115200);
    pinMode(0, INPUT_PULLUP);
    
    myWiFi.setAPName("TS_History_Node");
    myWiFi.addParameter("uid", "ThingsSentral User ID", defaultUserID, 15);
    myWiFi.setBootThresholds(3, 5);
    
    if (digitalRead(0) == LOW) {
        myWiFi.openPortal();
    } else {
        myWiFi.begin();
    }

    String activeUserID = String(myWiFi.getParameterValue("uid"));
    TS.begin(activeUserID); 

    // Sync Internet Time (Crucial for History timestamps)
    // Only attempt sync if WiFi connected successfully
    if (TS.isOnline()) {
        TS.syncTime("MYT-8"); 
    }
    
    // Initialize our counter with whatever is already saved offline
    localRecordCount = TS.History.count();
}

void loop() {
    myWiFi.loop();

    if (millis() - lastSampleTime >= sampleInterval) {
        lastSampleTime = millis();

        String tempSensorID = "0095312010101"; 
        String humSensorID  = "0095312010102"; 
        
        float currentTemp = random(200, 300) / 10.0; 
        float currentHum  = random(400, 800) / 10.0; 
        
        // 1. Stamp both data points (attaches Unix timestamp automatically)
        TS.History.stamp(tempSensorID, currentTemp);
        TS.History.stamp(humSensorID, currentHum);
        
        localRecordCount += 2;
        Serial.println("Records ready for upload: " + String(localRecordCount));

        // 2. Upload batch if we have 10 or more records
        if (localRecordCount >= 10) {
            if (TS.isOnline()) {
                Serial.println("Uploading batch history to server...");
                
                if (TS.History.upload()) {
                    Serial.println("Batch uploaded successfully! Buffer cleared.");
                    localRecordCount = 0; 
                } else {
                    Serial.println("Upload failed. Will retry next cycle.");
                }
            } else {
                Serial.println("Cannot upload batch: No internet.");
            }
        }
    }
}