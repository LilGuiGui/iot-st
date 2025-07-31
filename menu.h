#pragma once
#include "display.h"
#include "rtc_helper.h"

void showMenu() {
    lcdPrint(getTimeGreeting(), getCurrentTime());
    
    printRTCDebug();
    
    Serial.println("\nMenu:");
    Serial.println("1 - Test finger detection");
    Serial.println("2 - Simple enrollment test");
    Serial.println("3 - WiFi status");
    Serial.println("4 - Reset WiFi settings");
    Serial.println("5 - Disconnect WiFi");
    Serial.println("Enter command number:");
}