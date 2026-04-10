#include <WiFi.h> // Use <ESP8266WiFi.h> if using ESP8266
#include "ThingsSentral.h"

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
String myUserID = "12345"; // Your 5-digit User ID

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
}

void loop() {
    String tempSensorID = "0095312010101"; 
    String humSensorID  = "0095312010102"; 
    
    // Simulated sensor readings
    float currentTemp = random(200, 300) / 10.0; // 20.0 to 30.0
    float currentHum  = random(400, 800) / 10.0; // 40.0 to 80.0
    
    // 1. Store both readings locally first
    TS.Vault.add(tempSensorID, String(currentTemp));
    TS.Vault.add(humSensorID, String(currentHum));
    Serial.println("Saved Temp (" + String(currentTemp) + ") and Hum (" + String(currentHum) + ") to Vault.");

    // 2. Try to sync if we have an active connection
    if (TS.isOnline()) {
        Serial.println("Internet detected. Syncing vault to server...");
        // This will automatically loop through the file and send BOTH sensors' data
        TS.Vault.sync(); 
        Serial.println("Sync complete. Vault cleared.");
    } else {
        Serial.println("No internet. Data remains safely in Vault.");
    }
    
    delay(5000); // Read sensors every 5 seconds
}