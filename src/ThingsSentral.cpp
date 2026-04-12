#include "ThingsSentral.h"

void ThingsSentral::begin(String userID, String serverURL) {
    _userID = userID;
    _serverURL = serverURL;
    Command.parent = this;
    Vault.parent = this;
    History.parent = this;
    
    // Safety check: Ensure the file system actually starts
    if (!LittleFS.begin()) {
        Serial.println("TS: Error mounting LittleFS!");
    }
}

bool ThingsSentral::isOnline() {
    return WiFi.status() == WL_CONNECTED;
}

// FIX 1: Safe, localized networking (No Reentrancy Risk)
String ThingsSentral::_sendRequest(String url) {
    if (!isOnline()) return "";
    
    WiFiClient client;
    HTTPClient http;
    
    // Strict 5-second timeout to prevent infinite freezes
    http.setTimeout(5000); 
    
    http.begin(client, url);
    int httpCode = http.GET();
    String payload = "";
    
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY || httpCode == 404) {
            payload = http.getString();
        }
    } else {
        Serial.printf("TS: HTTP Error: %s\n", http.errorToString(httpCode).c_str());
    }
    
    // Cleanly destroy the socket to free up ESP memory slots
    http.end(); 
    return payload;
}

bool ThingsSentral::CommandModule::send(String sensorID, String value) {
    String url;
    url.reserve(parent->_serverURL.length() + parent->_userID.length() + sensorID.length() + value.length() + 30);
    url = parent->_serverURL;
    url += "/postlong?data=userid|";
    url += parent->_userID;
    url += "@";
    url += sensorID;
    url += "|";
    url += value;
    
    return parent->_sendRequest(url) != "";
}

ReadResult ThingsSentral::CommandModule::read(String sensorID) {
    ReadResult res = {0, "error", ""};
    String url = parent->_serverURL + "/ReadNode?Params=tokenid|" + parent->_userID + "@NodeId|" + sensorID;
    
    res.fullResponse = parent->_sendRequest(url);
    
    // Make sure response is long enough to actually contain data before parsing
    if (res.fullResponse.length() > 3) {
        int firstPipe = res.fullResponse.indexOf("|");
        if (firstPipe != -1) {
            int secondPipe = res.fullResponse.indexOf("|", firstPipe + 1);
            if (secondPipe != -1) {
                res.value = res.fullResponse.substring(firstPipe + 1, secondPipe);
            }
        }
    }
    return res;
}

void ThingsSentral::VaultModule::add(String sensorID, String value) {
    File f = LittleFS.open("/ts_vault.txt", "a");
    if (f) {
        f.printf("%s|%s\n", sensorID.c_str(), value.c_str());
        f.close();
    }
}

void ThingsSentral::VaultModule::sync() {
    if (!parent->isOnline() || !LittleFS.exists("/ts_vault.txt")) return;
    
    File f = LittleFS.open("/ts_vault.txt", "r");
    File temp = LittleFS.open("/ts_vault_temp.txt", "w"); // Create temp file for failed sends
    
    if (!f || !temp) return;

    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        int sep = line.indexOf('|');
        if (sep != -1) {
            String id = line.substring(0, sep);
            String val = line.substring(sep + 1);
            
            // If it FAILS to send, save it back to the temp file
            if (!parent->Command.send(id, val)) {
                temp.printf("%s|%s\n", id.c_str(), val.c_str());
            } else {
                delay(100); // Rate limit successful sends
            }
        }
    }
    
    f.close();
    temp.close();
    
    // Replace old vault with the temp vault (which only holds failed sends now)
    LittleFS.remove("/ts_vault.txt");
    LittleFS.rename("/ts_vault_temp.txt", "/ts_vault.txt");
}

void ThingsSentral::HistoryModule::stamp(String sensorID, float value) {
    File f = LittleFS.open("/ts_history.txt", "a"); 
    if (f) {
        f.printf("{\"id\":\"%s\",\"val\":%.2f,\"t\":%lld}\n", sensorID.c_str(), value, (long long)time(nullptr));
        f.close();
    }
}

int ThingsSentral::HistoryModule::count() {
    if (!LittleFS.exists("/ts_history.txt")) return 0;
    File f = LittleFS.open("/ts_history.txt", "r");
    if (!f) return 0;
    
    int lines = 0;
    while (f.available()) {
        if (f.read() == '\n') lines++; 
    }
    f.close();
    return lines;
}

bool ThingsSentral::HistoryModule::upload() {
    if (!parent->isOnline() || !LittleFS.exists("/ts_history.txt")) return false;

    File f = LittleFS.open("/ts_history.txt", "r");
    if (!f) return false;

    // FIX 3: Prevent Heap Fragmentation by pre-allocating memory
    String payload;
    payload.reserve(2048); 
    payload = "[";
    
    bool first = true;
    bool success = true;
    int estimatedEncodedLen = 1; // Account for the '[' bracket

    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        // FIX 5: Safe URL Length Math (assume worst-case 3x expansion for encoding)
        int lineMaxEncodedLen = line.length() * 3 + 1; 

        if (estimatedEncodedLen + lineMaxEncodedLen > 1500) {
            payload += "]"; 
            String url = parent->_serverURL + "/SendArray?DataToSend=" + parent->_urlEncode(payload);
            
            if (parent->_sendRequest(url) == "") {
                success = false;
                break; 
            }
            
            payload = "[";
            estimatedEncodedLen = 1;
            first = true;
        }

        if (!first) {
            payload += ",";
            estimatedEncodedLen++;
        }
        
        payload += line;
        estimatedEncodedLen += lineMaxEncodedLen;
        first = false;
    }

    if (success && payload.length() > 1) {
        payload += "]";
        String url = parent->_serverURL + "/SendArray?DataToSend=" + parent->_urlEncode(payload);
        if (parent->_sendRequest(url) == "") {
            success = false;
        }
    }
    f.close();

    if (success) {
        LittleFS.remove("/ts_history.txt");
        return true;
    }
    
    return false;
}

// FIX 2: Prevent the Infinite Time Loop of Death
void ThingsSentral::syncTime(const char* timezoneString, const char* ntpServer) {
    configTime(0, 0, ntpServer);
    setenv("TZ", timezoneString, 1);
    tzset();

    Serial.print("TS: Syncing NTP Time...");
    time_t now = time(nullptr);
    unsigned long start = millis();
    
    // Add a strict 15-second timeout
    while (now < 24 * 3600 && millis() - start < 15000) { 
        delay(500);
        Serial.print(".");
        now = time(nullptr);
    }
    
    if (now < 24 * 3600) {
        Serial.println(" Failed (Timeout).");
    } else {
        Serial.println(" Success!");
    }
}

// FIX 6: Unsigned char to prevent negative values breaking standard C functions
String ThingsSentral::_urlEncode(const String& str) {
    String encoded = "";
    encoded.reserve(str.length() * 2); // Minor optimization for the encoding string
    
    for (unsigned int i = 0; i < str.length(); i++) {
        unsigned char c = str[i]; 
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += (char)c;
        } else if (c == ' ') {
            encoded += "%20";
        } else {
            char buf[4];
            sprintf(buf, "%%%02X", c);
            encoded += buf;
        }
    }
    return encoded;
}

ThingsSentral TS;