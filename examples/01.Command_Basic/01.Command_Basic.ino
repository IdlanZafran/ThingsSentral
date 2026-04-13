/*
 * Example: Command Module (Live Send & Receive)
 * Author: Idlan Zafran Mohd Zaidie
 * * Description:
 * This sketch demonstrates the ThingsSentral Command module for real-time, 
 * two-way communication. It reads a local sensor and pushes the data live 
 * to the server, then queries the server for incoming instructions.
 * * It uses RapidBootWiFi to handle headless provisioning.
 */

#include <Arduino.h>
#include <RapidBootWiFi.h>
#include "ThingsSentral.h"

char defaultUserID[16] = "00953"; // Default fallback

// Non-blocking timer variables
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 10000; // 10 seconds

void setup() {
    Serial.begin(115200);
    pinMode(0, INPUT_PULLUP); // Setup the BOOT button for the Config Portal
    
    // 1. Initialize RapidBootWiFi
    myWiFi.setAPName("TS_Command_Node");
    myWiFi.addParameter("uid", "ThingsSentral User ID", defaultUserID, 15);
    myWiFi.setBootThresholds(3, 5);
    
    if (digitalRead(0) == LOW) {
        Serial.println("Button held! Opening Portal...");
        myWiFi.openPortal();
    } else {
        myWiFi.begin();
    }

    // 2. Initialize ThingsSentral using the ID saved from the web portal
    String activeUserID = String(myWiFi.getParameterValue("uid"));
    TS.begin(activeUserID); 
    
    Serial.println("System Ready. User ID: " + activeUserID);
}

void loop() {
    // Keep WiFi alive in the background
    myWiFi.loop();

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