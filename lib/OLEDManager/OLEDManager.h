#ifndef OLEDMANAGER_H
#define OLEDMANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

class OLEDManager {
public:
    static bool init();
    static bool isAvailable();
    static void clear();
    static void showStatus(const String& status);
    static void showSystemInfo();
    static void showWiFiInfo();
    static void showI2CInfo();
    static void updateDisplay();
    
private:
    static Adafruit_SSD1306* display;
    static bool available;
    static unsigned long lastUpdate;
    static const unsigned long UPDATE_INTERVAL = 2000; // Update every 2 seconds
    
    static void drawHeader();
    static void drawStatus(const String& status);
    static void drawSystemInfo();
    static void drawWiFiInfo();
    static void drawI2CInfo();
};

#endif
