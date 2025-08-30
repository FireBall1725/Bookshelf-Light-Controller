#ifndef FIRMWAREUPDATER_H
#define FIRMWAREUPDATER_H

#include <Arduino.h>
#include <Wire.h>
#include <SPIFFS.h>

class FirmwareUpdater {
public:
    static void init();
    static bool uploadFirmwareToSPIFFS(const uint8_t* firmwareData, size_t firmwareSize, const String& filename);
    static bool updateATtinyFirmware();
    static bool updateATtinyFirmwareFromSPIFFS(const String& filename = "attiny_firmware.hex");
    static bool checkATtinyVersion();
    static String getStoredFirmwareInfo(const String& filename = "attiny_firmware.hex");
    static bool deleteStoredFirmware(const String& filename = "attiny_firmware.hex");
    static String listStoredFirmwares();
    static bool firmwareExists(const String& filename = "attiny_firmware.hex");
    static size_t getFirmwareSize(const String& filename = "attiny_firmware.hex");
    
    // New methods for .bin package handling
    static bool uploadFirmwarePackage(const uint8_t* packageData, size_t packageSize, const String& filename);
    static bool extractFirmwarePackage(const String& packagePath);
    static bool parseFirmwareMetadata(const String& metadataPath, String& version, String& description, String& buildDate, String& board);
    static String getFirmwarePackageInfo(const String& filename = "firmware-v1.0.1.bin");
    static bool deleteFirmwarePackage(const String& filename = "firmware-v1.0.1.bin");
    static String listFirmwarePackages();
    static bool firmwarePackageExists(const String& filename = "firmware-v1.0.1.bin");
    
    // Enhanced package management
    static String generateFirmwareFilename(const String& version, const String& board);
    static bool checkDuplicateFirmware(const String& version, const String& board);
    static String getAllFirmwareInfo();

private:
    static const int ATTINY_ADDRESS = 0x50;
    static const char* FIRMWARE_DIR;
    
    static bool sendFirmwareLine(const String& line);
    static bool verifyFirmwareChecksum(const String& line);
    static void createFirmwareDirectory();
    static String getFirmwarePath(const String& filename);
    static int countHexLines(const String& filepath);
    static bool extractVersionFromHex(const String& hexContent, String& version, String& buildDate);
    static String parseHexLine(const String& hexLine);
    static bool isValidHexLine(const String& hexLine);
    static bool isValidDateFormat(const String& dateStr, char separator);
};

#endif
