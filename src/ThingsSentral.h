/**
 * ThingsSentral.h
 * An enterprise-grade library to connect ESP8266 and ESP32 devices to the ThingsSentral.io platform.
 * This library provides three core modules: 
 *      Command for live telemetry with Keep-Alive connection reuse, 
 *      Vault for safe offline data persistence using LittleFS, 
 *      and History for memory-safe, auto-chunked bulk data payloads. 
 * Hardened against memory leaks and socket exhaustion. 
 * @author Creator Idlan Zafran Mohd Zaidie
 * @version 1.0.0
 * @license MIT
 */


#ifndef ThingsSentral_h
#define ThingsSentral_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <time.h>

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
    void begin(String userID, String serverURL = "http://thingssentral.io");
    void syncTime(const char* timezoneString = "UTC0", const char* ntpServer = "pool.ntp.org");

    struct CommandModule {
        ThingsSentral* parent;
        ReadResult read(String sensorID);              
        bool send(String sensorID, String value);     
    } Command;

    struct VaultModule {
        ThingsSentral* parent;
        void add(String sensorID, String value);       
        void sync();                                   
    } Vault;

    struct HistoryModule {
        ThingsSentral* parent;
        void stamp(String sensorID, float value);      
        bool upload();                                 
        int count();
    } History;

    bool isOnline();
    
private:
    String _userID;
    String _serverURL;
    String _sendRequest(String url);
    String _urlEncode(const String& str);
};

extern ThingsSentral TS; 
#endif