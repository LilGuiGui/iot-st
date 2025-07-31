#pragma once
#include "config.h"

void initRTC() {
    // Use default Wire (pins 21, 22) - same as LCD
    Wire.begin(); // SDA=21, SCL=22 (default ESP32 I2C pins)
    
    if (!rtc.begin()) { // Using default Wire instead of rtcWire
        Serial.println("RTC ERROR: Couldn't find RTC");
        lcdPrint("RTC ERROR!", "Check connections");
        delay(3000);
        return;
    }
    
    if (rtc.lostPower()) {
        Serial.println("RTC lost power, setting time to compile time");
        // Set to compile time when first run or after power loss
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    
    Serial.println("RTC initialized successfully");
}

String getTimeGreeting() {
    DateTime now = rtc.now();
    int hour = now.hour();
    
    if (hour >= 5 && hour < 12) {
        return "Good Morning";
    } else if (hour >= 12 && hour < 17) {
        return "Good Afternoon";
    } else if (hour >= 17 && hour < 21) {
        return "Good Evening";
    } else {
        return "Good Night";
    }
}

String getCurrentTime() {
    DateTime now = rtc.now();
    
    char timeStr[20];
    sprintf(timeStr, "%02d:%02d:%02d %02d/%02d", 
            now.hour(), now.minute(), now.second(), 
            now.day(), now.month());
    
    return String(timeStr);
}

void printRTCDebug() {
    DateTime now = rtc.now();
    
    Serial.print("RTC Time: ");
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
}