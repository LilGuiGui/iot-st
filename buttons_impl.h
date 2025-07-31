#pragma once
#include "buttons.h"
#include "fingerprint.h"
#include "wifi_manager.h"
#include "rtc_helper.h"

int currentMenuItem = 0;
bool inMenu = false;
unsigned long lastButtonPress = 0;
unsigned long lastMenuUpdate = 0;

// Interrupt flags - volatile because they're modified in ISR
volatile bool leftPressed = false;
volatile bool selectPressed = false;
volatile bool rightPressed = false;
volatile unsigned long lastLeftInterrupt = 0;
volatile unsigned long lastSelectInterrupt = 0;
volatile unsigned long lastRightInterrupt = 0;

const unsigned long INTERRUPT_DEBOUNCE = 200; 

void IRAM_ATTR leftButtonISR() {
    unsigned long now = millis();
    if (now - lastLeftInterrupt > INTERRUPT_DEBOUNCE) {
        leftPressed = true;
        lastLeftInterrupt = now;
        Serial.println("LEFT INTERRUPT!");
    }
}

void IRAM_ATTR selectButtonISR() {
    unsigned long now = millis();
    if (now - lastSelectInterrupt > INTERRUPT_DEBOUNCE) {
        selectPressed = true;
        lastSelectInterrupt = now;
        Serial.println("SELECT INTERRUPT!");
    }
}

void IRAM_ATTR rightButtonISR() {
    unsigned long now = millis();
    if (now - lastRightInterrupt > INTERRUPT_DEBOUNCE) {
        rightPressed = true;
        lastRightInterrupt = now;
        Serial.println("RIGHT INTERRUPT!");
    }
}

void initButtons() {
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_SELECT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(BTN_LEFT), leftButtonISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_SELECT), selectButtonISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_RIGHT), rightButtonISR, FALLING);
    
    Serial.println("Buttons initialized with interrupts:");
    Serial.println("Left: GPIO " + String(BTN_LEFT));
    Serial.println("Select: GPIO " + String(BTN_SELECT));
    Serial.println("Right: GPIO " + String(BTN_RIGHT));
}

void handleButtons() {
    bool buttonHandled = false;
    
    if (leftPressed) {
        leftPressed = false; 
        Serial.println("Processing LEFT button");
        lastButtonPress = millis();
        buttonHandled = true;
        
        if (!inMenu) {
            Serial.println("Entering menu from LEFT...");
            enterMenu();
        } else {
            Serial.println("LEFT pressed in menu - navigate left");
            currentMenuItem = (currentMenuItem - 1 + menuItemCount) % menuItemCount;
            showMenuWithSelection();
        }
    }
    
    if (selectPressed) {
        selectPressed = false; 
        Serial.println("Processing SELECT button");
        lastButtonPress = millis();
        buttonHandled = true;
        
        if (!inMenu) {
            Serial.println("Entering menu from SELECT...");
            enterMenu();
        } else {
            Serial.println("SELECT pressed: " + menuItems[currentMenuItem]);
            executeMenuItem(currentMenuItem);
        }
    }
    
    if (rightPressed) {
        rightPressed = false; 
        Serial.println("Processing RIGHT button");
        lastButtonPress = millis();
        buttonHandled = true;
        
        if (!inMenu) {
            Serial.println("Entering menu from RIGHT...");
            enterMenu();
        } else {
            Serial.println("RIGHT pressed in menu - navigate right");
            currentMenuItem = (currentMenuItem + 1) % menuItemCount;
            showMenuWithSelection();
        }
    }
    
    if (!buttonHandled) {
        static unsigned long lastDebug = 0;
        if (millis() - lastDebug > 5000) {
            Serial.println("Status - InMenu:" + String(inMenu) + 
                          " MenuItem:" + String(currentMenuItem) +
                          " LastPress:" + String(millis() - lastButtonPress) + "ms ago");
            lastDebug = millis();
        }
    }
    
    if (inMenu && (millis() - lastButtonPress) > menuTimeout) {
        Serial.println("Menu timeout");
        exitMenu();
    }
    
    if (inMenu && (millis() - lastMenuUpdate) > 10000) {
        showMenuWithSelection();
        lastMenuUpdate = millis();
    }
}

void showMenuWithSelection() {
    String line1 = ">" + menuItems[currentMenuItem];
    String line2 = "(" + String(currentMenuItem + 1) + "/" + String(menuItemCount) + ") L<->R SEL";
    
    if (line1.length() > 16) {
        line1 = line1.substring(0, 15) + ">";
    }
    
    lcdPrint(line1, line2);
    lastMenuUpdate = millis();
    
    Serial.println("=== MENU ===");
    Serial.println(getTimeGreeting() + " - " + getCurrentTime());
    Serial.println("Selected: " + menuItems[currentMenuItem]);
    Serial.println("Use LEFT/RIGHT to navigate, SELECT to choose");
}

void executeMenuItem(int item) {
    inMenu = false; 
    
    switch(item) {
        case 0: 
            lcdPrint("Executing...", "Test Finger");
            delay(1000);
            testFingerDetection();
            break;
            
        case 1: 
            lcdPrint("Executing...", "Enroll Finger");
            delay(1000);
            simpleEnrollment();
            break;
            
        case 2:
            lcdPrint("Executing...", "WiFi Status");
            delay(1000);
            if (isWiFiConnected()) {
                lcdPrint("WiFi Connected", getWiFiIP());
                Serial.println("WiFi Status: Connected");
                Serial.println("IP Address: " + getWiFiIP());
            } else {
                lcdPrint("WiFi Disconnected", "Config mode active");
                Serial.println("WiFi Status: Disconnected or in config mode");
            }
            delay(3000);
            enterMenu(); 
            break;
            
        case 3: 
            lcdPrint("Executing...", "Reset WiFi");
            delay(1000);
            lcdPrint("Resetting WiFi", "Please wait...");
            Serial.println("Resetting WiFi configuration...");
            resetWiFiSettings();
            break;
            
        case 4: 
            lcdPrint("Executing...", "Disconnect WiFi");
            delay(1000);
            lcdPrint("Disconnecting", "WiFi...");
            disconnectWiFi();
            delay(2000);
            enterMenu(); 
            break;
            
        case 5: // Set RTC Time
            lcdPrint("Executing...", "Set RTC Time");
            delay(1000);
            Serial.println("Manual RTC time set (compile time)");
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
            lcdPrint("RTC Time Set", "To compile time");
            delay(2000);
            enterMenu(); 
            break;
            
        default:
            lcdPrint("Error", "Invalid selection");
            delay(2000);
            enterMenu(); 
            break;
    }
}

void enterMenu() {
    inMenu = true;
    currentMenuItem = 0; 
    lastButtonPress = millis();
    lastMenuUpdate = millis();
    showMenuWithSelection();
}

void exitMenu() {
    inMenu = false;
    lcdPrint(getTimeGreeting(), getCurrentTime());
    Serial.println("Menu timeout - showing status screen");
    printRTCDebug();
}
