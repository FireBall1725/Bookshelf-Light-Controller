#include "FirmwareUpdater.h"
#include "Logger.h"
#include <ArduinoJson.h>

// Static member initialization
const char* FirmwareUpdater::FIRMWARE_DIR = "/";

void FirmwareUpdater::init() {
    Logger::addEntry("FirmwareUpdater initialized");
}

void FirmwareUpdater::createFirmwareDirectory() {
    // SPIFFS doesn't support directories, so we store files in root
    Logger::addEntry("FirmwareUpdater using root directory for storage");
}

String FirmwareUpdater::getFirmwarePath(const String& filename) {
    // Use the same path pattern as the working HTML files
    if (filename.startsWith("/")) {
        return filename;
    }
    return "/" + filename;
}

bool FirmwareUpdater::uploadFirmwareToSPIFFS(const uint8_t* firmwareData, size_t firmwareSize, const String& filename) {
    String filepath = getFirmwarePath(filename);
    
    Logger::addEntry("Attempting to create firmware file: " + filepath);
    Logger::addEntry("SPIFFS total bytes: " + String(SPIFFS.totalBytes()));
    Logger::addEntry("SPIFFS used bytes: " + String(SPIFFS.usedBytes()));
    
    // Try to remove any existing file first
    if (SPIFFS.exists(filepath)) {
        SPIFFS.remove(filepath);
        Logger::addEntry("Removed existing firmware file: " + filepath);
    }
    
    File file = SPIFFS.open(filepath, "w");
    if (!file) {
        Logger::addEntry("Failed to create firmware file: " + filepath);
        return false;
    }
    
    size_t bytesWritten = file.write(firmwareData, firmwareSize);
    file.close();
    
    if (bytesWritten != firmwareSize) {
        Logger::addEntry("Failed to write firmware data. Expected: " + String(firmwareSize) + ", Written: " + String(bytesWritten));
        SPIFFS.remove(filepath); // Clean up partial file
        return false;
    }
    
    Logger::addEntry("Firmware uploaded to SPIFFS: " + filename + " (" + String(firmwareSize) + " bytes)");
    return true;
}

bool FirmwareUpdater::isValidDateFormat(const String& dateStr, char separator) {
    if (dateStr.length() != 10) {
        return false;
    }
    
    // Check separator positions
    if (dateStr.charAt(4) != separator || dateStr.charAt(7) != separator) {
        return false;
    }
    
    // Check if all other characters are digits
    for (int i = 0; i < dateStr.length(); i++) {
        if (i != 4 && i != 7) { // Skip separator positions
            if (dateStr.charAt(i) < '0' || dateStr.charAt(i) > '9') {
                return false;
            }
        }
    }
    
    // Basic validation: year should be reasonable, month 1-12, day 1-31
    int year = dateStr.substring(0, 4).toInt();
    int month = dateStr.substring(5, 7).toInt();
    int day = dateStr.substring(8, 10).toInt();
    
    if (year < 2000 || year > 2030) return false;
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;
    
    return true;
}

bool FirmwareUpdater::updateATtinyFirmware() {
    // This method now redirects to the SPIFFS version
    return updateATtinyFirmwareFromSPIFFS();
}

bool FirmwareUpdater::updateATtinyFirmwareFromSPIFFS(const String& filename) {
    if (!firmwareExists(filename)) {
        Logger::addEntry("No firmware file found: " + filename);
        return false;
    }
    
    String filepath = getFirmwarePath(filename);
    File file = SPIFFS.open(filepath, "r");
    if (!file) {
        Logger::addEntry("Failed to open firmware file: " + filepath);
        return false;
    }
    
    Logger::addEntry("Starting ATtiny firmware update from SPIFFS: " + filename);
    
    // Initialize I2C communication
    Wire.beginTransmission(ATTINY_ADDRESS);
    if (Wire.endTransmission() != 0) {
        Logger::addEntry("ATtiny not responding on I2C address 0x" + String(ATTINY_ADDRESS, HEX));
        file.close();
        return false;
    }
    
    // Send firmware update command
    Wire.beginTransmission(ATTINY_ADDRESS);
    Wire.write(0xFE); // Firmware update command
    if (Wire.endTransmission() != 0) {
        Logger::addEntry("Failed to send firmware update command");
        file.close();
        return false;
    }
    
    delay(100); // Give ATtiny time to prepare
    
    int lineCount = 0;
    int successCount = 0;
    
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        
        if (line.length() > 0 && line.startsWith(":")) {
            if (sendFirmwareLine(line)) {
                successCount++;
            }
            lineCount++;
            
            // Progress indicator every 100 lines
            if (lineCount % 100 == 0) {
                Logger::addEntry("Firmware update progress: " + String(lineCount) + " lines processed");
            }
            
            delay(1); // Small delay to prevent overwhelming the ATtiny
        }
    }
    
    file.close();
    
    // Send update complete command
    Wire.beginTransmission(ATTINY_ADDRESS);
    Wire.write(0xFF); // Update complete command
    Wire.endTransmission();
    
    delay(500); // Give ATtiny time to finalize
    
    Logger::addEntry("Firmware update completed. Lines: " + String(lineCount) + ", Success: " + String(successCount));
    
    return successCount == lineCount;
}

bool FirmwareUpdater::checkATtinyVersion() {
    Wire.beginTransmission(ATTINY_ADDRESS);
    Wire.write(0xFD); // Version check command
    if (Wire.endTransmission() != 0) {
        Logger::addEntry("Failed to send version check command");
        return false;
    }
    
    delay(100);
    
    Wire.requestFrom(ATTINY_ADDRESS, 32);
    if (Wire.available()) {
        String version = "";
        while (Wire.available()) {
            char c = Wire.read();
            if (c == '\0') break;
            version += c;
        }
        Logger::addEntry("ATtiny version: " + version);
        return true;
    }
    
    Logger::addEntry("No version response from ATtiny");
    return false;
}

String FirmwareUpdater::getStoredFirmwareInfo(const String& filename) {
    if (!firmwareExists(filename)) {
        return "Firmware not found: " + filename;
    }
    
    String filepath = getFirmwarePath(filename);
    File file = SPIFFS.open(filepath, "r");
    if (!file) {
        return "Failed to open firmware file";
    }
    
    size_t size = file.size();
    time_t lastModified = file.getLastWrite();
    
    // Read the entire hex content for parsing
    String hexContent = file.readString();
    file.close();
    
    // Extract version and build date from hex content
    String version, buildDate;
    bool versionFound = extractVersionFromHex(hexContent, version, buildDate);
    
    String info = "Filename: " + filename + "\n";
    info += "Size: " + String(size) + " bytes\n";
    info += "Modified: " + String(lastModified) + "\n";
    info += "Type: Intel HEX\n";
    info += "Lines: " + String(countHexLines(filepath));
    
    if (versionFound) {
        info += "\nVersion: " + version;
        info += "\nBuild Date: " + buildDate;
    }
    
    return info;
}

bool FirmwareUpdater::deleteStoredFirmware(const String& filename) {
    String filepath = getFirmwarePath(filename);
    
    if (SPIFFS.remove(filepath)) {
        Logger::addEntry("Deleted firmware file: " + filename);
        return true;
    } else {
        Logger::addEntry("Failed to delete firmware file: " + filename);
        return false;
    }
}

String FirmwareUpdater::listStoredFirmwares() {
    String list = "Stored Firmwares:\n";
    bool foundFiles = false;
    
    // SPIFFS doesn't support directories, so we need to check for specific firmware files
    // Check for common firmware filenames with leading slash to match storage pattern
    const char* firmwareFiles[] = {
        "/attiny_firmware.hex",
        "/firmware.hex",
        "/attiny.hex"
    };
    
    for (const char* filename : firmwareFiles) {
        if (SPIFFS.exists(filename)) {
            File file = SPIFFS.open(filename, "r");
            if (file) {
                // Remove leading slash for display
                String displayName = filename;
                if (displayName.startsWith("/")) {
                    displayName = displayName.substring(1);
                }
                list += "- " + displayName + " (" + String(file.size()) + " bytes)\n";
                file.close();
                foundFiles = true;
            }
        }
    }
    
    if (!foundFiles) {
        list += "No firmware files found";
    }
    
    return list;
}

bool FirmwareUpdater::firmwareExists(const String& filename) {
    return SPIFFS.exists(getFirmwarePath(filename));
}

size_t FirmwareUpdater::getFirmwareSize(const String& filename) {
    if (!firmwareExists(filename)) {
        return 0;
    }
    
    File file = SPIFFS.open(getFirmwarePath(filename), "r");
    if (!file) {
        return 0;
    }
    
    size_t size = file.size();
    file.close();
    return size;
}

bool FirmwareUpdater::sendFirmwareLine(const String& line) {
    Wire.beginTransmission(ATTINY_ADDRESS);
    
    // Send line length first
    Wire.write(line.length());
    
    // Send the actual line data
    for (int i = 0; i < line.length(); i++) {
        Wire.write(line.charAt(i));
    }
    
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    // Wait for acknowledgment
    delay(1);
    Wire.requestFrom(ATTINY_ADDRESS, 1);
    if (Wire.available()) {
        uint8_t ack = Wire.read();
        return (ack == 0x06); // ACK character
    }
    
    return false;
}

bool FirmwareUpdater::verifyFirmwareChecksum(const String& line) {
    if (line.length() < 11) return false; // Minimum HEX line length
    
    // Extract checksum from end of line
    String checksumStr = line.substring(line.length() - 2);
    uint8_t expectedChecksum = strtol(checksumStr.c_str(), NULL, 16);
    
    // Calculate checksum from data
    uint8_t calculatedChecksum = 0;
    for (int i = 1; i < line.length() - 2; i += 2) {
        String byteStr = line.substring(i, i + 2);
        uint8_t byte = strtol(byteStr.c_str(), NULL, 16);
        calculatedChecksum += byte;
    }
    calculatedChecksum = (0x100 - calculatedChecksum) & 0xFF;
    
    return calculatedChecksum == expectedChecksum;
}

int FirmwareUpdater::countHexLines(const String& filepath) {
    File file = SPIFFS.open(filepath, "r");
    if (!file) {
        return 0;
    }
    
    int lineCount = 0;
    while (file.available()) {
        String line = file.readStringUntil('\n');
        if (line.length() > 0 && line.startsWith(":")) {
            lineCount++;
        }
    }
    
    file.close();
    return lineCount;
}

bool FirmwareUpdater::extractVersionFromHex(const String& hexContent, String& version, String& buildDate) {
    // Initialize with default values
    version = "Unknown";
    buildDate = "Unknown";
    
    // Split content into lines
    int startPos = 0;
    int endPos = hexContent.indexOf('\n');
    
    while (endPos != -1) {
        String line = hexContent.substring(startPos, endPos);
        line.trim();
        
        if (isValidHexLine(line)) {
            String data = parseHexLine(line);
            if (data.length() > 0) {
                // Search for version string patterns
                if (data.indexOf("1.0.0") != -1) {
                    version = "1.0.0";
                } else if (data.indexOf("1.0.1") != -1) {
                    version = "1.0.1";
                } else if (data.indexOf("1.0.2") != -1) {
                    version = "1.0.2";
                } else if (data.indexOf("1.1.0") != -1) {
                    version = "1.1.0";
                }
                
                // Search for build date patterns (YYYY-MM-DD or YYYY/MM/DD)
                int datePos = data.indexOf("2024-");
                if (datePos != -1 && datePos + 10 <= data.length()) {
                    String potentialDate = data.substring(datePos, datePos + 10);
                    if (isValidDateFormat(potentialDate, '-')) {
                        buildDate = potentialDate;
                    }
                }
                
                // Alternative date format
                datePos = data.indexOf("2024/");
                if (datePos != -1 && datePos + 10 <= data.length()) {
                    String potentialDate = data.substring(datePos, datePos + 10);
                    if (isValidDateFormat(potentialDate, '/')) {
                        buildDate = potentialDate;
                    }
                }
            }
        }
        
        startPos = endPos + 1;
        endPos = hexContent.indexOf('\n', startPos);
    }
    
    // If we found at least one valid piece of info, return true
    return (version != "Unknown" || buildDate != "Unknown");
}

String FirmwareUpdater::parseHexLine(const String& hexLine) {
    if (!isValidHexLine(hexLine)) {
        return "";
    }
    
    // Extract data portion (skip :, length, address, keep data, skip checksum)
    if (hexLine.length() < 11) {
        return "";
    }
    
    // Data starts after :[length][address] and ends before [checksum]
    int dataStart = 9; // After :[2][4]
    int dataEnd = hexLine.length() - 2; // Before [2] checksum
    
    if (dataEnd <= dataStart) {
        return "";
    }
    
    String dataHex = hexLine.substring(dataStart, dataEnd);
    String asciiData = "";
    
    // Convert hex pairs to ASCII
    for (int i = 0; i < dataHex.length(); i += 2) {
        if (i + 1 < dataHex.length()) {
            String hexPair = dataHex.substring(i, i + 2);
            int asciiValue = strtol(hexPair.c_str(), NULL, 16);
            
            // Only include printable ASCII characters
            if (asciiValue >= 32 && asciiValue <= 126) {
                asciiData += (char)asciiValue;
            }
        }
    }
    
    return asciiData;
}

bool FirmwareUpdater::isValidHexLine(const String& hexLine) {
    // Check if line starts with : and has minimum length
    if (!hexLine.startsWith(":") || hexLine.length() < 11) {
        return false;
    }
    
    // Check if all characters are valid hex
    for (int i = 1; i < hexLine.length(); i++) {
        char c = hexLine.charAt(i);
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            return false;
        }
    }
    
    return true;
}

// New methods for .bin package handling
bool FirmwareUpdater::uploadFirmwarePackage(const uint8_t* packageData, size_t packageSize, const String& filename) {
    String filepath = getFirmwarePath(filename);
    
    Logger::addEntry("Attempting to create firmware package: " + filepath);
    Logger::addEntry("Package size: " + String(packageSize) + " bytes");
    
    // Try to remove any existing package first
    if (SPIFFS.exists(filepath)) {
        SPIFFS.remove(filepath);
        Logger::addEntry("Removed existing firmware package: " + filepath);
    }
    
    File file = SPIFFS.open(filepath, "w");
    if (!file) {
        Logger::addEntry("Failed to create firmware package file: " + filepath);
        return false;
    }
    
    size_t bytesWritten = file.write(packageData, packageSize);
    file.close();
    
    if (bytesWritten != packageSize) {
        Logger::addEntry("Failed to write package data. Expected: " + String(packageSize) + ", Written: " + String(bytesWritten));
        SPIFFS.remove(filepath); // Clean up partial file
        return false;
    }
    
    Logger::addEntry("Firmware package uploaded to SPIFFS: " + filename + " (" + String(packageSize) + " bytes)");
    
    // Extract the package contents
    return extractFirmwarePackage(filepath);
}

bool FirmwareUpdater::extractFirmwarePackage(const String& packagePath) {
    if (!SPIFFS.exists(packagePath)) {
        Logger::addEntry("Firmware package not found: " + packagePath);
        return false;
    }
    
    File packageFile = SPIFFS.open(packagePath, "r");
    if (!packageFile) {
        Logger::addEntry("Failed to open firmware package: " + packagePath);
        return false;
    }
    
    // Read the entire package into memory
    size_t packageSize = packageFile.size();
    uint8_t* packageData = new uint8_t[packageSize];
    packageFile.read(packageData, packageSize);
    packageFile.close();
    
    // Parse custom .bin format: [Magic][MetadataLength][Metadata][Firmware]
    if (packageSize < 10) { // Minimum: 5 magic + 4 length + 1 metadata
        Logger::addEntry("Package too small to be valid");
        delete[] packageData;
        return false;
    }
    
    // Check magic header "FLFW\0" (5 bytes)
    const char* expectedMagic = "FLFW\0";
    
    // Debug: Log the first 8 bytes
    String debugBytes = "First 8 bytes: ";
    for (int i = 0; i < 8; i++) {
        debugBytes += String(packageData[i], HEX) + " ";
    }
    Logger::addEntry(debugBytes);
    
    if (memcmp(packageData, expectedMagic, 5) != 0) {
        Logger::addEntry("Invalid package magic header");
        delete[] packageData;
        return false;
    }
    
    // Read metadata length (4 bytes, little-endian)
    uint32_t metadataLength = packageData[5] | (packageData[6] << 8) | 
                              (packageData[7] << 16) | (packageData[8] << 24);
    
    Logger::addEntry("Metadata length: " + String(metadataLength) + " bytes");
    
    if (metadataLength > 2048 || metadataLength == 0) {
        Logger::addEntry("Invalid metadata length: " + String(metadataLength));
        delete[] packageData;
        return false;
    }
    
    // Calculate positions
    size_t metadataStart = 9;  // After 5 magic + 4 length
    size_t metadataEnd = metadataStart + metadataLength;
    size_t firmwareStart = metadataEnd;
    
    if (firmwareStart >= packageSize) {
        Logger::addEntry("Package truncated - no firmware data");
        delete[] packageData;
        return false;
    }
    
    // Extract metadata
    String metadataJson = String((char*)&packageData[metadataStart], metadataLength);
    File metaFile = SPIFFS.open("/firmware.meta", "w");
    if (metaFile) {
        metaFile.print(metadataJson);
        metaFile.close();
        Logger::addEntry("Metadata extracted successfully");
    } else {
        Logger::addEntry("Failed to write metadata file");
        delete[] packageData;
        return false;
    }
    
    // Extract firmware hex
    size_t firmwareSize = packageSize - firmwareStart;
    File hexFile = SPIFFS.open("/firmware.hex", "w");
    if (hexFile) {
        hexFile.write(&packageData[firmwareStart], firmwareSize);
        hexFile.close();
        Logger::addEntry("Firmware extracted successfully: " + String(firmwareSize) + " bytes");
    } else {
        Logger::addEntry("Failed to write firmware file");
        delete[] packageData;
        return false;
    }
    
    delete[] packageData;
    
    Logger::addEntry("Firmware package extracted successfully");
    return true;
}

bool FirmwareUpdater::parseFirmwareMetadata(const String& metadataPath, String& version, String& description, String& buildDate, String& board) {
    if (!SPIFFS.exists(metadataPath)) {
        Logger::addEntry("Metadata file not found: " + metadataPath);
        return false;
    }
    
    File metaFile = SPIFFS.open(metadataPath, "r");
    if (!metaFile) {
        Logger::addEntry("Failed to open metadata file: " + metadataPath);
        return false;
    }
    
    String jsonContent = metaFile.readString();
    metaFile.close();
    
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, jsonContent);
    
    if (error) {
        Logger::addEntry("Failed to parse metadata JSON: " + String(error.c_str()));
        return false;
    }
    
    // Extract version info
    if (doc.containsKey("firmware") && doc["firmware"].containsKey("version")) {
        version = doc["firmware"]["version"].as<String>();
    } else {
        version = "Unknown";
    }
    
    if (doc.containsKey("firmware") && doc["firmware"].containsKey("description")) {
        description = doc["firmware"]["description"].as<String>();
    } else {
        description = "Unknown";
    }
    
    if (doc.containsKey("firmware") && doc["firmware"].containsKey("board")) {
        board = doc["firmware"]["board"].as<String>();
    } else {
        board = "Unknown";
    }
    
    if (doc.containsKey("build_info") && doc["build_info"].containsKey("timestamp")) {
        String timestamp = doc["build_info"]["timestamp"].as<String>();
        // Extract date part from ISO timestamp (YYYY-MM-DDTHH:MM:SS...)
        int dateEnd = timestamp.indexOf('T');
        if (dateEnd != -1) {
            buildDate = timestamp.substring(0, dateEnd);
        } else {
            buildDate = timestamp;
        }
    } else {
        buildDate = "Unknown";
    }
    
    return true;
}

String FirmwareUpdater::getFirmwarePackageInfo(const String& filename) {
    if (!firmwarePackageExists(filename)) {
        return "Firmware package not found: " + filename;
    }
    
    String filepath = getFirmwarePath(filename);
    File file = SPIFFS.open(filepath, "r");
    if (!file) {
        return "Failed to open firmware package";
    }
    
    size_t size = file.size();
    time_t lastModified = file.getLastWrite();
    file.close();
    
    String info = "Package: " + filename + "\n";
    info += "Size: " + String(size) + " bytes\n";
    info += "Modified: " + String(lastModified) + "\n";
    info += "Type: Firmware Package (.bin)\n";
    
                // Try to get metadata info if available
            if (SPIFFS.exists("/firmware.meta")) {
                String version, description, buildDate, board;
                if (parseFirmwareMetadata("/firmware.meta", version, description, buildDate, board)) {
                    info += "Version: " + version + "\n";
                    info += "Description: " + description + "\n";
                    info += "Build Date: " + buildDate + "\n";
                    info += "Board: " + board + "\n";
                }
            }
    
    return info;
}

bool FirmwareUpdater::deleteFirmwarePackage(const String& filename) {
    String filepath = getFirmwarePath(filename);
    
    // Also clean up extracted files
    if (SPIFFS.exists("/firmware.meta")) {
        SPIFFS.remove("/firmware.meta");
    }
    if (SPIFFS.exists("/firmware.hex")) {
        SPIFFS.remove("/firmware.hex");
    }
    
    if (SPIFFS.remove(filepath)) {
        Logger::addEntry("Deleted firmware package: " + filename);
        return true;
    } else {
        Logger::addEntry("Failed to delete firmware package: " + filename);
        return false;
    }
}

String FirmwareUpdater::listFirmwarePackages() {
    String list = "Firmware Packages:\n";
    bool foundFiles = false;
    
    // Check for common firmware package filenames
    const char* packageFiles[] = {
        "/firmware-v1.0.1.bin",
        "/firmware-v1.0.0.bin",
        "/firmware-v1.0.2.bin",
        "/firmware.bin"
    };
    
    for (const char* filename : packageFiles) {
        if (SPIFFS.exists(filename)) {
            File file = SPIFFS.open(filename, "r");
            if (file) {
                String displayName = filename;
                if (displayName.startsWith("/")) {
                    displayName = displayName.substring(1);
                }
                list += "- " + displayName + " (" + String(file.size()) + " bytes)\n";
                file.close();
                foundFiles = true;
            }
        }
    }
    
    if (!foundFiles) {
        list += "No firmware packages found";
    }
    
    return list;
}

bool FirmwareUpdater::firmwarePackageExists(const String& filename) {
    return SPIFFS.exists(getFirmwarePath(filename));
}

// Enhanced package management methods
String FirmwareUpdater::generateFirmwareFilename(const String& version, const String& board) {
    String filename = "firmware-";
    filename += version;
    filename += "-";
    filename += board;
    filename += ".bin";
    return filename;
}

bool FirmwareUpdater::checkDuplicateFirmware(const String& version, const String& board) {
    String filename = generateFirmwareFilename(version, board);
    return firmwarePackageExists(filename);
}

String FirmwareUpdater::getAllFirmwareInfo() {
    String info = "";
    bool foundFiles = false;
    
    // Scan SPIFFS for all .bin files
    File root = SPIFFS.open("/");
    if (!root) {
        return "Failed to open SPIFFS root";
    }
    
    if (!root.isDirectory()) {
        root.close();
        return "SPIFFS root is not a directory";
    }
    
    File file = root.openNextFile();
    while (file) {
        String filename = file.name();
        if (filename.endsWith(".bin")) {
            if (foundFiles) {
                info += "\n---\n";
            }
            
            // Get basic file info
            size_t size = file.size();
            time_t lastModified = file.getLastWrite();
            file.close();
            
            info += "Filename: " + filename + "\n";
            info += "Size: " + String(size) + " bytes\n";
            info += "Modified: " + String(lastModified) + "\n";
            
            // Try to get metadata info
            String metadataPath = "/" + filename;
            metadataPath.replace(".bin", ".meta");
            if (SPIFFS.exists(metadataPath)) {
                String version, description, buildDate, board;
                if (parseFirmwareMetadata(metadataPath, version, description, buildDate, board)) {
                    info += "Version: " + version + "\n";
                    info += "Description: " + description + "\n";
                    info += "Build Date: " + buildDate + "\n";
                    info += "Board: " + board + "\n";
                }
            }
            
            foundFiles = true;
        }
        file = root.openNextFile();
    }
    root.close();
    
    if (!foundFiles) {
        info = "No firmware packages found";
    }
    
    return info;
}
