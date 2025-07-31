#pragma once
#include "config.h"
#include "display.h"
#include "menu.h"

bool awaitForFingerPlace(const String& str) {
    lcdPrint(str, "Silahkan Letakkan jari Anda");
    Serial.println(str);
    while (finger.getImage() != FINGERPRINT_OK) delay(100);
    return true;
}

bool awaitForFingerRemove(const String& str) {
    lcdPrint(str, "Silahkan Angkat Jari Anda");
    Serial.println(str);
    while (finger.getImage() != FINGERPRINT_NOFINGER) delay(100);
    return true;
}

void testFingerDetection() {
    lcdPrint("Sanity test", "...");
    Serial.println("10s Max delay...");

    int count = 0;

    for (int i = 0; i < 100; i++) {
        uint8_t p = finger.getImage();

        if (p == FINGERPRINT_OK) {
            count++;
            lcdPrint("OK!", "Count: " + String(count));
            Serial.println("\u2713 OK!");
            delay(500);
        } 
        else if (p == FINGERPRINT_NOFINGER) {
            lcd.setCursor(15, 1);
            lcd.print(".");
            Serial.print(".");
            delay(100);
        } 
        else {
            showError("err:", p);
            delay(100);
        }
    }

    lcdPrint("complete!", "count: " + String(count));
    Serial.print("count: "); Serial.println(count);

    delay(3000);
    showMenu();
}

int getNextID() {
    lcdPrint("Scanning IDs...", "Mohon Tunggu...");
    Serial.println("lookup available ID slots...");
    
    for (int i = 1; i <= 300; i++) {
        if (i % 50 == 0) {
            lcdPrint("Scanning IDs...", "ID: " + String(i) + "/300");
            Serial.println("Checking ID: " + String(i));
        }
        
        uint8_t p = finger.loadModel(i);
        
        if (p != FINGERPRINT_OK) {
            Serial.println("Found available ID: " + String(i));
            return i;
        }
        
        delay(10);
    }
    
    Serial.println("No ID slots found!");
    return -1; 
}

int getEnrolledCount() {
    int count = 0;
    for (int i = 1; i <= 300; i++) {
        uint8_t p = finger.loadModel(i);
        if (p == FINGERPRINT_OK) {
            count++;
        }
        delay(5);
    }
    return count;
}

void cleanSensorReading() {
    finger.getImage();
    delay(100);
}

void simpleEnrollment() {
    lcdPrint("Enrolling", "Memulai...");
    Serial.println("Starting Enrollment");
    delay(1000);

    int enrollID = getNextID();
    
    if (enrollID == -1) {
        lcdPrint("ERROR!", "No available slots");
        Serial.println("ERROR: No available ID slots (1-300 all occupied)");
        delay(3000);
        showMenu();
        return;
    }
    
    lcdPrint("ID: " + String(enrollID), "Total: " + String(getEnrolledCount()) + "/300");
    Serial.println("Using ID #" + String(enrollID) + " for enrollment");
    delay(2000);

    cleanSensorReading();

    showStep(1, "Letakan Jari Anda");
    lcdPrint("Step 1/5", "Tekan jari dengan");
    lcdPrintLine(1, "tekanan sedang");
    
    while (finger.getImage() != FINGERPRINT_OK) {
        delay(50);
    }

    lcdPrint("Fingerprint captured!", "Converting...");
    Serial.println("\u2713 Image1 captured");

    uint8_t p = finger.image2Tz(1);
    if (p != FINGERPRINT_OK) {
        showError("Image1 convert fail:", p);
        delay(3000);
        showMenu();
        return;
    }

    lcdPrint("Convert OK!", "Angkat jari...");
    Serial.println("\u2713 Image1 converted");
    delay(1000);

    showStep(2, "Angkat jari Anda");
    while (finger.getImage() != FINGERPRINT_NOFINGER) {
        delay(50);
    }

    lcdPrint("Jari diangkat", "Siap untuk step 3");
    Serial.println("\u2713 Finger removed");
    delay(1000);

    showStep(3, "Letakkan sama persis");
    lcdPrint("Step 3/5", "Pastikan posisi");
    lcdPrintLine(1, "SAMA seperti tadi");
    
    while (finger.getImage() != FINGERPRINT_OK) {
        delay(50);
    }

    lcdPrint("Fingerprint captured!", "Converting...");
    Serial.println("\u2713 Image2 captured");

    p = finger.image2Tz(2);
    if (p != FINGERPRINT_OK) {
        showError("Image2 convert fail:", p);
        delay(3000);
        showMenu();
        return;
    }

    lcdPrint("Convert OK!", "Creating model...");
    Serial.println("\u2713 Image2 converted");
    delay(1000);

    showStep(4, "Creating model...");
    Serial.println("Creating fingerprint model...");
    p = finger.createModel();
    
    if (p == FINGERPRINT_ENROLLMISMATCH) {
        lcdPrint("ERROR 11!", "Fingerprints don't");
        lcdPrintLine(1, "match. Try again.");
        Serial.println("ERROR 11: FINGERPRINT_ENROLLMISMATCH - Images don't match well enough");
        Serial.println("Tips: Ensure same finger position, clean sensor, consistent pressure");
        delay(4000);
        showMenu();
        return;
    } else if (p != FINGERPRINT_OK) {
        showError("Model creation fail:", p);
        delay(3000);
        showMenu();
        return;
    }

    lcdPrint("Model OK!", "Storing template...");
    Serial.println("\u2713 Model created successfully");
    delay(1000);

    showStep(5, "Storing Template...");
    Serial.println("Storing to ID #" + String(enrollID) + "...");
    p = finger.storeModel(enrollID);
    if (p != FINGERPRINT_OK) {
        showError("Storage fail:", p);
        delay(3000);
        showMenu();
        return;
    }

    lcdPrint("SUCCESS!", "ID #" + String(enrollID));
    Serial.println("\u2713 Enrolled successfully to ID #" + String(enrollID));
    Serial.println("Enrollment complete!");

    delay(3000);
    showMenu();
}