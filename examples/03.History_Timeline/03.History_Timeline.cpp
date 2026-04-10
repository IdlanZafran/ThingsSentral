#include <WiFi.h> // Use <ESP8266WiFi.h> if using ESP8266
#include "ThingsSentral.h"

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

String myUserID = "00953"; 

void setup() {
    Serial.begin(115200);
    
    // 1. Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");

    // 2. Initialize ThingsSentral
    TS.begin(myUserID); 

    // 3. Sync Internet Time (Crucial for History timestamps!)
    // Change "MYT-8" to your actual timezone format
    TS.syncTime("MYT-8"); 
}

void loop() {
    String tempSensorID = "0095312010101"; 
    String humSensorID  = "0095312010102"; 
    
    // Simulated sensor readings
    float currentTemp = random(200, 300) / 10.0; 
    float currentHum  = random(400, 800) / 10.0; 
    
    // 1. Stamp both data points (attaches Unix timestamp automatically)
    TS.History.stamp(tempSensorID, currentTemp);
    TS.History.stamp(humSensorID, currentHum);
    
    // 2. Check how many total records we have saved
    int recordsSaved = TS.History.count();
    Serial.println("Saved records: " + String(recordsSaved));

    // 3. If we have collected 10 records (e.g., 5 temp + 5 hum), upload them
    if (recordsSaved >= 10) {
        if (TS.isOnline()) {
            Serial.println("Uploading batch history to server...");
            bool uploadSuccess = TS.History.upload();
            
            if (uploadSuccess) {
                Serial.println("Batch uploaded successfully! Buffer cleared.");
            }
        } else {
            Serial.println("Cannot upload batch: No internet.");
        }
    }
    
    delay(2000); 
}