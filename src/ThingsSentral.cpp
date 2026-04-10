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

ThingsSentral TS;