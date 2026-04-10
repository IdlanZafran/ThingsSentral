/**
 * ThingsSentral.h
 * 
 * A library to connect ESP8266 and ESP32 devices to the ThingsSentral.io platform.
 * This library provides three core modules: Command for live sending and reading of telemetry data, Vault for offline data persistence using LittleFS, and History for timestamped bulk data payloads.
 * 
 * @author Idlan Zafran Mohd Zaidie
 * @version 1.0.0
 * @license MIT
 */


#ifndef ThingsSentral_h
#define ThingsSentral_h

#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

#include <LittleFS.h>

struct ReadResult {
    int httpCode;
    String value;
    String fullResponse;
};

class ThingsSentral {
public:
    // Pass your 5-digit User ID here
    void begin(String userID, String serverURL = "http://thingssentral.io");

    // MODULE 1: COMMAND (Live Send/Read)
    struct CommandModule {
        ThingsSentral* parent;
        ReadResult read(String sensorID);              // Read from 13-digit ID
        bool send(String sensorID, String value);     // Send to 13-digit ID
    } Command;

    // MODULE 2: VAULT (Offline Safety)
    struct VaultModule {
        ThingsSentral* parent;
        void add(String sensorID, String value);       // Store data locally
        void sync();                                   // Push stored data to web
    } Vault;

    // MODULE 3: HISTORY (Time-Series)
    struct HistoryModule {
        ThingsSentral* parent;
        void stamp(String sensorID, float value);      // Mark with timestamp
        bool upload();                                 // Send bulk JSON
        int count();
    } History;

    bool isOnline();
    
private:
    String _userID;
    String _serverURL;
    String _sendRequest(String url);
};

extern ThingsSentral TS; 
#endif