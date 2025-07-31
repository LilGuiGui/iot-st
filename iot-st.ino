#include "config.h"
#include "display.h"
#include "fingerprint.h"
#include "menu.h"
#include "wifi_manager.h"
#include "rtc_helper.h"
#include "buttons.h"
#include "buttons_impl.h"

// Device definitions
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger(&mySerial);
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

// WiFi Manager variable definitions (to prevent multiple definition errors)
AsyncWebServer* wifiServer = nullptr;
String wifi_ssid = "";
String wifi_pass = "";
String wifi_ip = "";
String wifi_gateway = "";

// Network configuration
IPAddress localIP;
IPAddress localGateway;
IPAddress subnet(255, 255, 0, 0);

// File paths
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";

// Parameters
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

// Connection timeout
const long connectionTimeout = 10000;

void setup() {
  delay(2000);
  Serial.begin(9600);
  delay(1000);

  // Initialize I2C first for both LCD and RTC
  Wire.begin(); // SDA=21, SCL=22 (default ESP32 I2C pins)
  delay(100);

  lcd.init();
  lcd.backlight();
  lcdPrint("Starting...", "");
  Serial.println("Hewwo");

  // Initialize buttons
  lcdPrint("Init Buttons...", "");
  initButtons();
  
  // ADD THIS DEBUG LINE to see button states immediately
  Serial.println("Initial button check:");
  Serial.println("Left: " + String(digitalRead(BTN_LEFT)));
  Serial.println("Select: " + String(digitalRead(BTN_SELECT))); 
  Serial.println("Right: " + String(digitalRead(BTN_RIGHT)));
  
  delay(500);

  lcdPrint("Init RTC...", "");
  initRTC();
  delay(1000);

  mySerial.begin(57600, SERIAL_8N1, 16, 17);
  delay(1000);

  lcdPrint("Checking sensor", "");
  Serial.println("Assessing Sensor status");

  if (finger.verifyPassword()) {
    lcdPrint("SENSOR OK!", "Max: " + String(finger.capacity));
    Serial.println("SENSOR OK!");
    finger.getParameters();
    Serial.print("Max Templates: "); Serial.println(finger.capacity);
    Serial.print("Security: "); Serial.println(finger.security_level);
    delay(2000);
  } else {
    showError("No sensor found");
    while (1) delay(1000);
  }

  lcdPrint("WiFi Setup", "Initializing...");
  
  if (initWiFiManager()) {
    lcdPrint("WiFi Connected!", getWiFiIP());
    Serial.println("WiFi Manager: Connected successfully");
  } else {
    lcdPrint("WiFi Config Mode", "Connect to ESP-WIFI-MANAGER");
    Serial.println("WiFi Manager: In configuration mode");
  }
  
  delay(3000);
  
  // Show initial status screen
  lcdPrint(getTimeGreeting(), getCurrentTime());
  Serial.println("\n=== SYSTEM READY ===");
  Serial.println("Press any button to access menu");
  Serial.println("Or use serial commands (1-6)");
}

void loop() {
    // Handle button interrupts
    handleButtons();
    
    // Update display if not in menu
    if (!inMenu) {
        static unsigned long lastTimeUpdate = 0;
        if (millis() - lastTimeUpdate > 10000) { // Update time every 10 seconds
            lcdPrint(getTimeGreeting(), getCurrentTime());
            lastTimeUpdate = millis();
        }
    }
    
    // Small delay to prevent overwhelming the processor
    delay(50);
}