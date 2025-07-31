/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete instructions at https://RandomNerdTutorials.com/esp32-wi-fi-manager-asyncwebserver/
  Permission is hereby granted, free of charge, to any person obtaining 
  a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or 
  substantial portions of the Software.
*********/

#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "LittleFS.h"

//arduino-cli lib install "ESP Async WebServer"
//arduino-cli lib install "AsyncTCP"
//arduino-cli lib install "LittleFS_esp32"

// Function declarations
bool initWiFiManager();
void handleWiFiManager();
bool isWiFiConnected();
String getWiFiIP();
void resetWiFiSettings();
void disconnectWiFi();

String readWiFiFile(const char* path);
void writeWiFiFile(const char* path, const char* message);
bool connectToWiFi();
void startConfigMode();
void startNormalMode();

// External variable declarations (defined in main file)
extern String wifi_ssid;
extern String wifi_pass;
extern String wifi_ip;
extern String wifi_gateway;
extern AsyncWebServer* wifiServer;

// Network configuration (extern declarations)
extern IPAddress localIP;
extern IPAddress localGateway;
extern IPAddress subnet;

// File paths (extern declarations)
extern const char* ssidPath;
extern const char* passPath;
extern const char* ipPath;
extern const char* gatewayPath;

// Parameters (extern declarations)
extern const char* PARAM_INPUT_1;
extern const char* PARAM_INPUT_2;
extern const char* PARAM_INPUT_3;
extern const char* PARAM_INPUT_4;

// Connection timeout (extern declaration)
extern const long connectionTimeout;

// Function implementations
inline String readWiFiFile(const char* path) {
    Serial.printf("WiFi: Reading file: %s\r\n", path);
    
    File file = LittleFS.open(path);
    if(!file || file.isDirectory()){
        Serial.println("WiFi: Failed to open file for reading");
        return String();
    }
    
    String fileContent;
    while(file.available()){
        fileContent = file.readStringUntil('\n');
        break;     
    }
    file.close();
    return fileContent;
}

inline void writeWiFiFile(const char* path, const char* message) {
    Serial.printf("WiFi: Writing file: %s\r\n", path);
    
    File file = LittleFS.open(path, FILE_WRITE);
    if(!file){
        Serial.println("WiFi: Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("WiFi: File written");
    } else {
        Serial.println("WiFi: Write failed");
    }
    file.close();
}

inline bool connectToWiFi() {
    if(wifi_ssid == "" || wifi_ip == ""){
        Serial.println("WiFi: Undefined SSID or IP address");
        return false;
    }
    
    WiFi.mode(WIFI_STA);
    localIP.fromString(wifi_ip.c_str());
    localGateway.fromString(wifi_gateway.c_str());
    
    if (!WiFi.config(localIP, localGateway, subnet)){
        Serial.println("WiFi: STA Failed to configure");
        return false;
    }
    
    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
    Serial.println("WiFi: Connecting...");
    
    unsigned long startTime = millis();
    
    while(WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime >= connectionTimeout) {
            Serial.println("WiFi: Connection timeout");
            return false;
        }
        delay(100);
    }
    
    Serial.println("WiFi: Connected! IP: " + WiFi.localIP().toString());
    return true;
}

inline void startNormalMode() {
    Serial.println("WiFi: Starting normal mode");
    
    // Create server if it doesn't exist
    if (!wifiServer) {
        wifiServer = new AsyncWebServer(80);
    }
    
    // Route for status page
    wifiServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = "<html><body>";
        html += "<h1>ESP32 Status</h1>";
        html += "<p>WiFi Connected: " + WiFi.localIP().toString() + "</p>";
        html += "<p>Fingerprint System Active</p>";
        html += "</body></html>";
        request->send(200, "text/html", html);
    });
    
    // Route for WiFi status API
    wifiServer->on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"connected\": true,";
        json += "\"ip\": \"" + WiFi.localIP().toString() + "\",";
        json += "\"ssid\": \"" + WiFi.SSID() + "\"";
        json += "}";
        request->send(200, "application/json", json);
    });
    
    wifiServer->begin();
}

inline void startConfigMode() {
    Serial.println("WiFi: Starting configuration mode");
    
    // Start Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("WiFi: AP IP address: ");
    Serial.println(IP);
    
    // Create server if it doesn't exist
    if (!wifiServer) {
        wifiServer = new AsyncWebServer(80);
    }
    
    // Configuration page
    wifiServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = "<html><head><title>WiFi Manager</title></head><body>";
        html += "<h1>WiFi Configuration</h1>";
        html += "<form action='/' method='POST'>";
        html += "<p><label>SSID:</label><br><input type='text' name='ssid' required></p>";
        html += "<p><label>Password:</label><br><input type='password' name='pass' required></p>";
        html += "<p><label>IP Address:</label><br><input type='text' name='ip' placeholder='192.168.1.100' required></p>";
        html += "<p><label>Gateway:</label><br><input type='text' name='gateway' placeholder='192.168.1.1' required></p>";
        html += "<p><input type='submit' value='Connect'></p>";
        html += "</form></body></html>";
        request->send(200, "text/html", html);
    });
    
    // Handle configuration form submission
    wifiServer->on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
        int params = request->params();
        for(int i = 0; i < params; i++){
            const AsyncWebParameter* p = request->getParam(i);
            if(p->isPost()){
                if (p->name() == PARAM_INPUT_1) {
                    wifi_ssid = p->value().c_str();
                    Serial.println("WiFi: SSID set to: " + wifi_ssid);
                    writeWiFiFile(ssidPath, wifi_ssid.c_str());
                }
                if (p->name() == PARAM_INPUT_2) {
                    wifi_pass = p->value().c_str();
                    Serial.println("WiFi: Password updated");
                    writeWiFiFile(passPath, wifi_pass.c_str());
                }
                if (p->name() == PARAM_INPUT_3) {
                    wifi_ip = p->value().c_str();
                    Serial.println("WiFi: IP set to: " + wifi_ip);
                    writeWiFiFile(ipPath, wifi_ip.c_str());
                }
                if (p->name() == PARAM_INPUT_4) {
                    wifi_gateway = p->value().c_str();
                    Serial.println("WiFi: Gateway set to: " + wifi_gateway);
                    writeWiFiFile(gatewayPath, wifi_gateway.c_str());
                }
            }
        }
        request->send(200, "text/plain", "Configuration saved! ESP will restart and connect to: " + wifi_ip);
        delay(3000);
        ESP.restart();
    });
    
    wifiServer->begin();
}

inline bool initWiFiManager() {
    Serial.println("WiFi: Initializing WiFi Manager");
    
    // Initialize LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("WiFi: LittleFS mount failed");
        return false;
    }
    Serial.println("WiFi: LittleFS mounted successfully");
    
    // Load saved configuration
    wifi_ssid = readWiFiFile(ssidPath);
    wifi_pass = readWiFiFile(passPath);
    wifi_ip = readWiFiFile(ipPath);
    wifi_gateway = readWiFiFile(gatewayPath);
    
    Serial.println("WiFi: Loaded config - SSID: " + wifi_ssid + ", IP: " + wifi_ip);
    
    // Try to connect with saved credentials
    if (connectToWiFi()) {
        startNormalMode();
        return true;
    } else {
        startConfigMode();
        return false;
    }
}

inline void handleWiFiManager() {
    // Empty implementation - kept for compatibility
}

inline bool isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

inline String getWiFiIP() {
    if (isWiFiConnected()) {
        return WiFi.localIP().toString();
    }
    return "";
}

inline void resetWiFiSettings() {
    Serial.println("WiFi: Resetting configuration");
    
    // Clear saved credentials
    LittleFS.remove(ssidPath);
    LittleFS.remove(passPath);
    LittleFS.remove(ipPath);
    LittleFS.remove(gatewayPath);
    
    // Clear variables
    wifi_ssid = "";
    wifi_pass = "";
    wifi_ip = "";
    wifi_gateway = "";
    
    Serial.println("WiFi: Configuration reset, restarting...");
    ESP.restart();
}

inline void disconnectWiFi() {
    Serial.println("WiFi: Disconnecting");
    WiFi.disconnect();
}