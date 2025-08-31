#include "I2CScanner.h"
#include "Logger.h"

// Initialize with correct I2C pins
int I2CScanner::SDA_PIN = 6;  // SDA pin
int I2CScanner::SCL_PIN = 7;  // SCL pin

void I2CScanner::init() {
    init(SDA_PIN, SCL_PIN);
}

void I2CScanner::init(int sdaPin, int sclPin) {
    SDA_PIN = sdaPin;
    SCL_PIN = sclPin;
    
    Wire.begin(SDA_PIN, SCL_PIN);
    
    Logger::addEntry("I2C initialized on SDA:GPIO" + String(SDA_PIN) + ", SCL:GPIO" + String(SCL_PIN));
}

String I2CScanner::scan() {
    Logger::addEntry("Starting I2C bus scan...");
    int deviceCount = 0;
    String result = "I2C Scan Results:\n";
    
    for (byte address = SCAN_START; address < SCAN_END; address++) {
        if (testAddress(address)) {
            deviceCount++;
            String deviceInfo = getDeviceInfo(address);
            Logger::addEntry(deviceInfo);
            result += deviceInfo + "\n";
        }
    }
    
    if (deviceCount == 0) {
        Logger::addEntry("No I2C devices found");
        result = "No I2C devices found";
    } else {
        Logger::addEntry("I2C scan complete. Found " + String(deviceCount) + " device(s)");
        result += "\nTotal devices found: " + String(deviceCount);
    }
    
    return result;
}

String I2CScanner::scanWithDetails() {
    Logger::addEntry("Starting detailed I2C bus scan...");
    int deviceCount = 0;
    String result = "Detailed I2C Scan Results:\n";
    
    // Test common OLED addresses first
    byte commonAddresses[] = {0x3C, 0x3D, 0x27, 0x20, 0x48, 0x68, 0x76, 0x77};
    result += "Testing common addresses first:\n";
    
    for (byte addr : commonAddresses) {
        if (testAddress(addr)) {
            deviceCount++;
            String deviceInfo = getDeviceInfo(addr);
            Logger::addEntry("Common device found: " + deviceInfo);
            result += "✓ " + deviceInfo + "\n";
        }
    }
    
    // Full scan
    result += "\nFull address scan:\n";
    for (byte address = SCAN_START; address < SCAN_END; address++) {
        if (testAddress(address)) {
            deviceCount++;
            String deviceInfo = getDeviceInfo(address);
            Logger::addEntry(deviceInfo);
            result += "✓ " + deviceInfo + "\n";
        }
    }
    
    if (deviceCount == 0) {
        Logger::addEntry("No I2C devices found");
        result = "No I2C devices found\n\nTroubleshooting tips:\n";
        result += "1. Check wiring (SDA:GPIO" + String(SDA_PIN) + ", SCL:GPIO" + String(SCL_PIN) + ")\n";
        result += "2. Verify power to OLED display\n";
        result += "3. Check pull-up resistors (4.7kΩ recommended)\n";
        result += "4. Try different I2C addresses\n";
        result += "5. Verify OLED is I2C compatible\n";
    } else {
        Logger::addEntry("Detailed I2C scan complete. Found " + String(deviceCount) + " device(s)");
        result += "\nTotal devices found: " + String(deviceCount);
    }
    
    return result;
}

bool I2CScanner::testAddress(byte address) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    return (error == 0);
}

void I2CScanner::sendCommand(byte command) {
    Logger::addEntry("Sending I2C command: 0x" + String(command, HEX));
    // This method can be extended to send specific commands to I2C devices
}

String I2CScanner::getDeviceInfo(byte address) {
    String deviceInfo = "I2C device found at address 0x" + String(address, HEX) + " (0x" + String(address, HEX) + ")";
    
    // Try to identify common devices
    if (address == 0x3C || address == 0x3D) {
        deviceInfo += " - Likely OLED Display";
    } else if (address == 0x48) {
        deviceInfo += " - Likely ADS1115 ADC";
    } else if (address == 0x68) {
        deviceInfo += " - Likely RTC (DS3231/DS1307)";
    } else if (address == 0x76 || address == 0x77) {
        deviceInfo += " - Likely BME280/BMP280";
    } else if (address == 0x39 || address == 0x29) {
        deviceInfo += " - Likely TCS3200/TCS230";
    } else if (address == 0x23 || address == 0x5C) {
        deviceInfo += " - Likely BH1750 Light Sensor";
    } else if (address == 0x27) {
        deviceInfo += " - Likely LCD Display (PCF8574)";
    } else if (address == 0x20) {
        deviceInfo += " - Likely I/O Expander (MCP23008)";
    } else if (address == 0x50) {
        deviceInfo += " - Likely EEPROM (24C32/24C64)";
    }
    
    return deviceInfo;
}
