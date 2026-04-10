/*
 * Example: Command Module (Live Send & Receive)
 * Author: Idlan Zafran Mohd Zaidie
 * * Description:
 * This sketch demonstrates the ThingsSentral Command module for real-time, 
 * two-way communication. It reads a local sensor and pushes the data live 
 * to the server. Immediately after, it queries the server for incoming 
 * instructions (like a button press on a dashboard) to control a local relay.
 * * Sample Uses:
 * - Smart Home applications where you need to report room temperature while simultaneously listening for a command to turn on a smart plug.
 * - Live industrial dashboards that require instant, up-to-the-second telemetry without any batching delays.
 * - Bidirectional remote-control vehicles or drones where real-time command execution is critical.
 */

#include <Arduino.h>
#include <WiFi.h> // Use <ESP8266WiFi.h> if using ESP8266
#include "ThingsSentral.h"

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

String myUserID = "00953"; // MUST match the first 5 digits of your Sensor IDs

// Non-blocking timer variables
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 10000; // 10 seconds

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
}

void loop() {
    // Check if it has been 10 seconds since the last run
    if (millis() - lastSendTime >= sendInterval) {
        lastSendTime = millis(); // Reset timer

        if (TS.isOnline()) {
            // --- SENDING LIVE DATA ---
            String tempSensorID = "0095312010101"; 
            String humSensorID  = "0095312010102"; 
            
            String temperature = "24.5";
            String humidity = "60.2";
            
            if (TS.Command.send(tempSensorID, temperature)) {
                Serial.println("Temperature sent successfully!");
            }
            if (TS.Command.send(humSensorID, humidity)) {
                Serial.println("Humidity sent successfully!");
            }

            // --- READING LIVE COMMANDS ---
            String switch1ID = "0095312010103"; 
            String switch2ID = "0095312010104"; 
            
            ReadResult res1 = TS.Command.read(switch1ID);
            ReadResult res2 = TS.Command.read(switch2ID);
            
            Serial.println("Switch 1 Value: " + res1.value); 
            Serial.println("Switch 2 Value: " + res2.value); 
        }
    }
    
}