#include "ThingsSentral.h"

void ThingsSentral::begin(String userID, String serverURL) {
    _userID = userID;
    _serverURL = serverURL;
    Command.parent = this;
    Vault.parent = this;
    History.parent = this;
    LittleFS.begin();
}

bool ThingsSentral::isOnline() {
    return WiFi.status() == WL_CONNECTED;
}

String ThingsSentral::_sendRequest(String url) {
    if (!isOnline()) return "";
    WiFiClient client;
    HTTPClient http;
    http.begin(client, url);
    int httpCode = http.GET();
    String payload = (httpCode > 0) ? http.getString() : "";
    http.end();
    return payload;
}

bool ThingsSentral::CommandModule::send(String sensorID, String value) {
    // Format: http://thingssentral.io/postlong?data=userid|00953@0095312010101|25.0
    String url = parent->_serverURL + "/postlong?data=userid|" + parent->_userID;
    url += "@" + sensorID + "|" + value;
    return parent->_sendRequest(url) != "";
}

// COMMAND: Read logic
ReadResult ThingsSentral::CommandModule::read(String sensorID) {
    ReadResult res = {0, "error", ""};
    String url = parent->_serverURL + "/ReadNode?Params=tokenid|" + parent->_userID + "@NodeId|" + sensorID;
    
    res.fullResponse = parent->_sendRequest(url);
    int firstPipe = res.fullResponse.indexOf("|");
    int secondPipe = res.fullResponse.indexOf("|", firstPipe + 1);
    
    if (firstPipe != -1 && secondPipe != -1) {
        res.value = res.fullResponse.substring(firstPipe + 1, secondPipe);
    }
    return res;
}

// VAULT: Offline persistence logic
void ThingsSentral::VaultModule::add(String sensorID, String value) {
    File f = LittleFS.open("/ts_vault.txt", "a");
    if (f) {
        f.printf("%s|%s\n", sensorID.c_str(), value.c_str());
        f.close();
    }
}

void ThingsSentral::VaultModule::sync() {
    if (!parent->isOnline()) return;
    File f = LittleFS.open("/ts_vault.txt", "r");
    if (!f) return;

    while (f.available()) {
        String line = f.readStringUntil('\n');
        int sep = line.indexOf('|');
        if (sep != -1) {
            parent->Command.send(line.substring(0, sep), line.substring(sep + 1));
        }
    }
    f.close();
    LittleFS.remove("/ts_vault.txt");
}

int ThingsSentral::HistoryModule::count() {
    if (!LittleFS.exists("/ts_history.json")) return 0;

    File f = LittleFS.open("/ts_history.json", "r");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, f);
    f.close();

    if (error) return 0;
    return doc.size(); // Returns the number of elements in the JSON array
}

// HISTORY: Time-Series JSON formatting
void ThingsSentral::HistoryModule::stamp(String sensorID, float value) {
    JsonDocument doc; // Dynamically sizes memory for the JSON

    // If a history file already exists, load it into the document
    if (LittleFS.exists("/ts_history.json")) {
        File f = LittleFS.open("/ts_history.json", "r");
        deserializeJson(doc, f);
        f.close();
    }

    // Add the new data point to the JSON Array
    JsonObject obj = doc.add<JsonObject>();
    obj["id"] = sensorID;
    obj["val"] = value;
    obj["t"] = time(nullptr);

    // Save the updated JSON array back to the flash memory
    File f = LittleFS.open("/ts_history.json", "w");
    serializeJson(doc, f);
    f.close();
}

bool ThingsSentral::HistoryModule::upload() {
    // Abort if offline or if there is no history file to send
    if (!parent->isOnline() || !LittleFS.exists("/ts_history.json")) return false;

    // Read the entire JSON array into a string
    File f = LittleFS.open("/ts_history.json", "r");
    String jsonStr = f.readString();
    f.close();

    // Safely encode the JSON string before adding it to the URL
    String encodedJson = parent->_urlEncode(jsonStr);
    
    // Construct the Bulk Upload URL
    String url = parent->_serverURL + "/SendArray?DataToSend=" + encodedJson;
    
    // If the server responds successfully, delete the local file to start fresh
    if (parent->_sendRequest(url) != "") {
        LittleFS.remove("/ts_history.json");
        return true;
    }
    
    return false;
}

// Fetches the real time from the internet
void ThingsSentral::syncTime(const char* timezoneString, const char* ntpServer) {
    configTime(0, 0, ntpServer);
    setenv("TZ", timezoneString, 1);
    tzset();

    Serial.print("TS: Syncing NTP Time...");
    time_t now = time(nullptr);
    // Wait until the time is valid (greater than the year 1970)
    while (now < 24 * 3600) { 
        delay(500);
        Serial.print(".");
        now = time(nullptr);
    }
    Serial.println(" Success!");
}

// Helper function to safely encode JSON strings for HTTP GET URLs
String ThingsSentral::_urlEncode(const String& str) {
    String encoded = "";
    for (unsigned int i = 0; i < str.length(); i++) {
        char c = str[i];
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
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