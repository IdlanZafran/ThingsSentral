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
    if (TS.isOnline()) {
        
        // --- SENDING DATA FOR TWO SENSORS ---
        String tempSensorID = "0095312010101"; // Temperature ID
        String humSensorID  = "0095312010102"; // Humidity ID
        
        String temperature = "24.5";
        String humidity = "60.2";
        
        // Send first sensor
        if (TS.Command.send(tempSensorID, temperature)) {
            Serial.println("Temperature sent successfully!");
        }
        
        // Send second sensor
        if (TS.Command.send(humSensorID, humidity)) {
            Serial.println("Humidity sent successfully!");
        }

        // --- READING DATA FOR TWO SWITCHES ---
        String switch1ID = "0095312010103"; // Switch 1 ID
        String switch2ID = "0095312010104"; // Switch 2 ID
        
        ReadResult res1 = TS.Command.read(switch1ID);
        ReadResult res2 = TS.Command.read(switch2ID);
        
        Serial.println("Switch 1 Value: " + res1.value); 
        Serial.println("Switch 2 Value: " + res2.value); 
        
        // Example logic for multiple switches
        // if (res1.value == "1") { /* Turn on Relay 1 */ }
        // if (res2.value == "1") { /* Turn on Relay 2 */ }
    }
    
    delay(10000); // Wait 10 seconds before next cycle
}