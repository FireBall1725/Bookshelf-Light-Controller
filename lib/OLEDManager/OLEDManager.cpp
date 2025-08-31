#include "OLEDManager.h"
#include "Logger.h"
#include <WiFi.h>

// Static member initialization
Adafruit_SSD1306* OLEDManager::display = nullptr;
bool OLEDManager::available = false;
unsigned long OLEDManager::lastUpdate = 0;

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

bool OLEDManager::init() {
    if (available) {
        return true; // Already initialized
    }
    
    Logger::addEntry("Initializing OLED display...");
    
    // Check if OLED is present at address 0x3C
    Wire.beginTransmission(SCREEN_ADDRESS);
    byte error = Wire.endTransmission();
    
    if (error != 0) {
        Logger::addEntry("OLED not detected at address 0x3C");
        return false;
    }
    
    Logger::addEntry("OLED detected at address 0x3C, initializing...");
    
    // Create display object
    display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    
    if (!display->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Logger::addEntry("Failed to initialize OLED display");
        delete display;
        display = nullptr;
        return false;
    }
    
    // Clear display and show startup message
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println("ESP32 Starting...");
    display->display();
    
    available = true;
    lastUpdate = millis();
    
    Logger::addEntry("OLED display initialized successfully");
    return true;
}

bool OLEDManager::isAvailable() {
    return available && display != nullptr;
}

void OLEDManager::clear() {
    if (isAvailable()) {
        display->clearDisplay();
        display->display();
    }
}

void OLEDManager::showStatus(const String& status) {
    if (!isAvailable()) return;
    
    display->clearDisplay();
    drawHeader();
    drawStatus(status);
    display->display();
}

void OLEDManager::showSystemInfo() {
    if (!isAvailable()) return;
    
    display->clearDisplay();
    drawHeader();
    drawSystemInfo();
    display->display();
}

void OLEDManager::showWiFiInfo() {
    if (!isAvailable()) return;
    
    display->clearDisplay();
    drawHeader();
    drawWiFiInfo();
    display->display();
}

void OLEDManager::showI2CInfo() {
    if (!isAvailable()) return;
    
    display->clearDisplay();
    drawHeader();
    drawI2CInfo();
    display->display();
}

void OLEDManager::showDefaultDisplay() {
    if (!isAvailable()) return;
    
    display->clearDisplay();
    drawHeader();
    drawDefaultInfo();
    display->display();
}

void OLEDManager::updateDisplay() {
    if (!isAvailable()) return;
    
    // Only update every UPDATE_INTERVAL milliseconds
    if (millis() - lastUpdate < UPDATE_INTERVAL) {
        return;
    }
    
    lastUpdate = millis();
    
    // Show clean default display
    showDefaultDisplay();
}

void OLEDManager::drawHeader() {
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println("ESP32-C3 Controller");
    display->drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
}

void OLEDManager::drawStatus(const String& status) {
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 15);
    
    // Split status into lines if it's too long
    String remaining = status;
    int yPos = 15;
    
    while (remaining.length() > 0 && yPos < SCREEN_HEIGHT - 10) {
        String line = remaining;
        if (remaining.length() > 20) {
            line = remaining.substring(0, 20);
            remaining = remaining.substring(20);
        } else {
            remaining = "";
        }
        
        display->setCursor(0, yPos);
        display->println(line);
        yPos += 10;
    }
}

void OLEDManager::drawSystemInfo() {
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    
    int yPos = 15;
    
    // Uptime - simplified format
    unsigned long uptime = millis();
    unsigned long hours = uptime / 3600000;
    uptime %= 3600000;
    unsigned long minutes = uptime / 60000;
    uptime %= 60000;
    unsigned long seconds = uptime / 1000;
    
    display->setCursor(0, yPos);
    display->print("Uptime: ");
    if (hours > 0) {
        display->print(hours);
        display->print("h ");
    }
    display->print(minutes);
    display->print("m ");
    display->print(seconds);
    display->print("s");
    yPos += 10;
    
    // WiFi status
    display->setCursor(0, yPos);
    display->print("WiFi: ");
    if (WiFi.status() == WL_CONNECTED) {
        display->print("Connected");
    } else {
        display->print("Disconnected");
    }
    yPos += 10;
    
    // IP Address
    if (WiFi.status() == WL_CONNECTED) {
        display->setCursor(0, yPos);
        display->print("IP: ");
        display->print(WiFi.localIP().toString());
    }
    
    // WiFi status
    display->setCursor(0, yPos);
    display->print("WiFi: ");
    if (WiFi.status() == WL_CONNECTED) {
        display->print("Connected");
    } else {
        display->print("Disconnected");
    }
    yPos += 10;
    
    // IP Address
    if (WiFi.status() == WL_CONNECTED) {
        display->setCursor(0, yPos);
        display->print("IP: ");
        display->print(WiFi.localIP().toString());
    }
}

void OLEDManager::drawWiFiInfo() {
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    
    int yPos = 15;
    
    // WiFi Status
    display->setCursor(0, yPos);
    display->print("WiFi Status: ");
    if (WiFi.status() == WL_CONNECTED) {
        display->print("Connected");
    } else {
        display->print("Disconnected");
    }
    yPos += 10;
    
    if (WiFi.status() == WL_CONNECTED) {
        // SSID
        display->setCursor(0, yPos);
        display->print("SSID: ");
        display->print(WiFi.SSID());
        yPos += 10;
        
        // IP Address
        display->setCursor(0, yPos);
        display->print("IP: ");
        display->print(WiFi.localIP().toString());
        yPos += 10;
        
        // Signal Strength
        display->setCursor(0, yPos);
        display->print("RSSI: ");
        display->print(WiFi.RSSI());
        display->print(" dBm");
        yPos += 10;
        
        // MAC Address
        display->setCursor(0, yPos);
        display->print("MAC: ");
        String mac = WiFi.macAddress();
        display->print(mac.substring(0, 8));
        yPos += 10;
        display->setCursor(0, yPos);
        display->print("      ");
        display->print(mac.substring(9));
    }
}

void OLEDManager::drawI2CInfo() {
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    
    int yPos = 15;
    
    // I2C Status
    display->setCursor(0, yPos);
    display->print("I2C Bus Status");
    yPos += 10;
    
    // Scan for devices
    int deviceCount = 0;
    for (byte addr = 1; addr < 128; addr++) {
        Wire.beginTransmission(addr);
        byte error = Wire.endTransmission();
        if (error == 0) {
            deviceCount++;
            
            if (yPos < SCREEN_HEIGHT - 10) {
                display->setCursor(0, yPos);
                display->print("0x");
                if (addr < 16) display->print("0");
                display->print(addr, HEX);
                
                // Identify common devices
                if (addr == 0x3C || addr == 0x3D) {
                    display->print(" - OLED");
                } else if (addr == 0x48) {
                    display->print(" - ADC");
                } else if (addr == 0x68) {
                    display->print(" - RTC");
                }
                
                yPos += 10;
            }
        }
    }
    
    // Total device count
    if (yPos < SCREEN_HEIGHT - 10) {
        display->setCursor(0, yPos);
        display->print("Total: ");
        display->print(deviceCount);
        display->print(" devices");
    }
}

void OLEDManager::drawDefaultInfo() {
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    
    int yPos = 15;
    
    // Clean uptime display
    unsigned long uptime = millis();
    unsigned long hours = uptime / 3600000;
    uptime %= 3600000;
    unsigned long minutes = uptime / 60000;
    uptime %= 60000;
    unsigned long seconds = uptime / 1000;
    
    display->setCursor(0, yPos);
    display->print("Uptime: ");
    if (hours > 0) {
        display->print(hours);
        display->print("h ");
    }
    display->print(minutes);
    display->print("m ");
    display->print(seconds);
    display->print("s");
    yPos += 12;
    
    // WiFi status
    display->setCursor(0, yPos);
    display->print("WiFi: ");
    if (WiFi.status() == WL_CONNECTED) {
        display->print("Connected");
    } else {
        display->print("Disconnected");
    }
    yPos += 12;
    
    // IP Address (only if connected)
    if (WiFi.status() == WL_CONNECTED) {
        display->setCursor(0, yPos);
        display->print("IP: ");
        display->print(WiFi.localIP().toString());
        yPos += 12;
        
        // Signal strength
        display->setCursor(0, yPos);
        display->print("Signal: ");
        display->print(WiFi.RSSI());
        display->print(" dBm");
    }
}
