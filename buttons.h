#pragma once
#include "config.h"
#include "display.h"

#define BTN_LEFT 25
#define BTN_SELECT 26
#define BTN_RIGHT 27

enum ButtonState {
    BTN_RELEASED,
    BTN_PRESSED,
    BTN_HELD
};

const String menuItems[] = {
    "Test Finger",
    "Enroll Finger", 
    "WiFi Status",
    "Reset WiFi",
    "Disconnect WiFi",
    "Set RTC Time"
};
const int menuItemCount = 6;

extern int currentMenuItem;
extern bool inMenu;
extern unsigned long lastButtonPress;
extern unsigned long lastMenuUpdate;

const unsigned long debounceDelay = 50;
const unsigned long menuTimeout = 30000;

void initButtons();
ButtonState readButton(int pin, bool &lastState, unsigned long &lastDebounce);
void handleButtons();
void showMenuWithSelection();
void executeMenuItem(int item);
void enterMenu();
void exitMenu();
