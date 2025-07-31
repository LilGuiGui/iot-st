#pragma once
#include "config.h"

void lcdPrintLine(uint8_t line, const String& text) {
    lcd.setCursor(0, line);
    lcd.print("                ");
    lcd.setCursor(0, line);
    lcd.print(text);
}

void lcdPrint(const String& line1, const String& line2 = "") {
    lcd.clear();
    lcdPrintLine(0, line1);
    lcdPrintLine(1, line2);
}

void showError(const String& msg, uint8_t code = 0xFF) {
    lcdPrint("Error:", msg);
    Serial.print("ERROR: ");
    Serial.println(msg);
    if (code != 0xFF) {
        Serial.print("Code: ");
        Serial.println(code);
    }
}

void showStep(int step, const String& msg) {
    lcdPrint("Step " + String(step) + "/5", msg);
}
