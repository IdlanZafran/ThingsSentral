/*
 * Example: Vault Module with Dynamic Flash-Saving Routing
 * Author: Idlan Zafran Mohd Zaidie
 * * Description:
 * This sketch demonstrates an advanced implementation of the Vault module.
 * To preserve the lifespan of the ESP's flash memory, it attempts to send 
 * sensor readings live to the server first. If the send fails (no internet 
 * or server down), it automatically falls back to storing the data offline 
 * in LittleFS. When the connection is restored, it syncs the offline cache.
 * * Sample Uses:
 * - Remote weather stations or agricultural sensors located in areas with spotty cellular or WiFi coverage.
 * - Mobile asset trackers (e.g., vehicle fleet monitors) that frequently pass through network dead zones.
 * - Critical industrial monitors where momentary network drops are common, but zero data loss is required.
 */

#include <Arduino.h>
#include <WiFi.h> // Use <ESP8266WiFi.h> if using ESP8266
#include "ThingsSentral.h"

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

String myUserID = "00953"; 

// Non-blocking timer variables
unsigned long lastReadTime = 0;
const unsigned long readInterval = 5000; // 5 seconds

void setup() {
    Serial.begin(115200);
    
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");

    TS.begin(myUserID); 
}

void loop() {
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
            // This runs instantly if the vault is empty, so it's safe to call here.
            TS.Vault.sync(); 
        }
        
        Serial.println("---------------------------------");
    }
}