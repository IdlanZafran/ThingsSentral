/*
 * Example: Vault Module with Dynamic Flash-Saving Routing
 * Author: Idlan Zafran Mohd Zaidie
 * * Description:
 * This sketch tries to send data live. If the internet drops, it automatically 
 * falls back to storing the data offline in LittleFS. When the connection 
 * is restored by RapidBootWiFi, it syncs the offline cache.
 */

#include <Arduino.h>
#include <RapidBootWiFi.h>
#include "ThingsSentral.h"

char defaultUserID[16] = "00953"; 

unsigned long lastReadTime = 0;
const unsigned long readInterval = 5000; // 5 seconds

void setup() {
    Serial.begin(115200);
    pinMode(0, INPUT_PULLUP);
    
    myWiFi.setAPName("TS_Vault_Node");
    myWiFi.addParameter("uid", "ThingsSentral User ID", defaultUserID, 15);
    myWiFi.setBootThresholds(3, 5);
    
    if (digitalRead(0) == LOW) {
        myWiFi.openPortal();
    } else {
        myWiFi.begin();
    }

    String activeUserID = String(myWiFi.getParameterValue("uid"));
    TS.begin(activeUserID); 
}

void loop() {
    myWiFi.loop();

    if (millis() - lastReadTime >= readInterval) {
        lastReadTime = millis();

        String tempSensorID = "0095312010101"; 
        String humSensorID  = "0095312010102"; 
        
        // Simulated sensor readings converted to Strings
        String currentTemp = String(random(200, 300) / 10.0); 
        String currentHum  = String(random(400, 800) / 10.0); 
        
        bool tempSent = false;
        bool humSent = false;

        // 1. Try to send live data directly to the server first
        if (TS.isOnline()) {
            tempSent = TS.Command.send(tempSensorID, currentTemp);
            humSent  = TS.Command.send(humSensorID, currentHum);
        }

        // 2. Fallback routing for Temperature
        if (tempSent) {
            Serial.println("Temp (" + currentTemp + ") sent live! Skipped flash memory.");
        } else {
            TS.Vault.add(tempSensorID, currentTemp);
            Serial.println("Temp live send failed. Saved to offline Vault.");
        }

        // 3. Fallback routing for Humidity
        if (humSent) {
            Serial.println("Hum (" + currentHum + ") sent live! Skipped flash memory.");
        } else {
            TS.Vault.add(humSensorID, currentHum);
            Serial.println("Hum live send failed. Saved to offline Vault.");
        }

        // 4. Housekeeping: Sync any old data trapped in the Vault
        if (TS.isOnline()) {
            TS.Vault.sync(); 
        }
        
        Serial.println("---------------------------------");
    }
}