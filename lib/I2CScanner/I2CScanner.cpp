#include "I2CScanner.h"
#include "Logger.h"

void I2CScanner::init() {
    Wire.begin(SDA_PIN, SCL_PIN);
    Logger::addEntry("I2C initialized on SDA:GPIO" + String(SDA_PIN) + ", SCL:GPIO" + String(SCL_PIN));
}

String I2CScanner::scan() {
    Logger::addEntry("Starting I2C bus scan...");
    int deviceCount = 0;
    
    for (byte address = SCAN_START; address < SCAN_END; address++) {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();
        
        if (error == 0) {
            deviceCount++;
            String deviceInfo = getDeviceInfo(address);
            Logger::addEntry(deviceInfo);
        }
    }
    
    if (deviceCount == 0) {
        Logger::addEntry("No I2C devices found");
    } else {
        Logger::addEntry("I2C scan complete. Found " + String(deviceCount) + " device(s)");
    }
    
    return "I2C scan complete. Found " + String(deviceCount) + " device(s)";
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
