#pragma once

#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Wire.h>

extern HardwareSerial mySerial;
extern Adafruit_Fingerprint finger;
extern LiquidCrystal_I2C lcd;
extern RTC_DS3231 rtc;
// Removed rtcWire - now using default Wire for both LCD and RTC

// Button navigation variables
extern int currentMenuItem;
extern bool inMenu;
extern unsigned long lastButtonPress;
extern unsigned long lastMenuUpdate;
