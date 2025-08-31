#include "mock_firmware_updater.h"
#include "mock_logger.h"

// Mock FirmwareUpdater implementation for testing
const char* FirmwareUpdater::FIRMWARE_DIR = "/";

// Mock storage for testing
static std::map<String, std::string> mockFirmwareFiles;
static std::map<String, std::string> mockFirmwarePackages;

void FirmwareUpdater::init() {
    Logger::addEntry("FirmwareUpdater initialized");
    // Clear mock storage
    mockFirmwareFiles.clear();
    mockFirmwarePackages.clear();
}

void FirmwareUpdater::createFirmwareDirectory() {
    Logger::addEntry("FirmwareUpdater using root directory for storage");
}

String FirmwareUpdater::getFirmwarePath(const String& filename) {
    if (filename.startsWith("/")) {
        return filename;
    }
    return "/" + filename;
}

bool FirmwareUpdater::uploadFirmwareToSPIFFS(const uint8_t* firmwareData, size_t firmwareSize, const String& filename) {
    String filepath = getFirmwarePath(filename);
    
    Logger::addEntry("Attempting to create firmware file: " + filepath);
    
    // Store in mock storage
    std::string data(reinterpret_cast<const char*>(firmwareData), firmwareSize);
    mockFirmwareFiles[filepath] = data;
    
    Logger::addEntry("Firmware uploaded to SPIFFS: " + filename + " (" + std::to_string(firmwareSize).c_str() + " bytes)");
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
    return updateATtinyFirmwareFromSPIFFS();
}

bool FirmwareUpdater::updateATtinyFirmwareFromSPIFFS(const String& filename) {
    if (!firmwareExists(filename)) {
        Logger::addEntry("No firmware file found: " + filename);
        return false;
    }
    
    Logger::addEntry("Updating ATtiny firmware from: " + filename);
    return true; // Mock success
}

bool FirmwareUpdater::checkATtinyVersion() {
    Logger::addEntry("Checking ATtiny version");
    return true; // Mock success
}

String FirmwareUpdater::getStoredFirmwareInfo(const String& filename) {
    if (!firmwareExists(filename)) {
        return "Firmware not found";
    }
    
    String filepath = getFirmwarePath(filename);
    size_t size = getFirmwareSize(filename);
    
    String info = "Filename: " + filename + "\n";
    info += "Size: " + String(std::to_string(size).c_str()) + " bytes\n";
    info += "Type: Intel HEX\n";
    
    // Try to extract version info if it's a hex file
    if (filename.endsWith(".hex")) {
        auto it = mockFirmwareFiles.find(filepath);
        if (it != mockFirmwareFiles.end()) {
            String hexContent = String(it->second.c_str());
            String version, buildDate;
            if (extractVersionFromHex(hexContent, version, buildDate)) {
                info += "\nVersion: " + version;
                info += "\nBuild Date: " + buildDate;
            }
        }
    }
    
    return info;
}

bool FirmwareUpdater::deleteStoredFirmware(const String& filename) {
    String filepath = getFirmwarePath(filename);
    
    if (mockFirmwareFiles.erase(filepath) > 0) {
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
    
    for (const auto& pair : mockFirmwareFiles) {
        String filename = pair.first;
        if (filename.startsWith("/")) {
            filename = filename.substring(1);
        }
        list += "- " + filename + " (" + std::to_string(pair.second.length()).c_str() + " bytes)\n";
        foundFiles = true;
    }
    
    if (!foundFiles) {
        list += "No firmware files found";
    }
    
    return list;
}

bool FirmwareUpdater::firmwareExists(const String& filename) {
    return mockFirmwareFiles.find(getFirmwarePath(filename)) != mockFirmwareFiles.end();
}

size_t FirmwareUpdater::getFirmwareSize(const String& filename) {
    if (!firmwareExists(filename)) {
        return 0;
    }
    
    String filepath = getFirmwarePath(filename);
    auto it = mockFirmwareFiles.find(filepath);
    if (it != mockFirmwareFiles.end()) {
        return it->second.length();
    }
    
    return 0;
}

bool FirmwareUpdater::sendFirmwareLine(const String& line) {
    Logger::addEntry("Sending firmware line to ATtiny: " + line);
    return true; // Mock success
}

bool FirmwareUpdater::verifyFirmwareChecksum(const String& line) {
    // Simple mock checksum verification
    return line.length() > 0 && line.startsWith(":");
}

int FirmwareUpdater::countHexLines(const String& filepath) {
    auto it = mockFirmwareFiles.find(filepath);
    if (it == mockFirmwareFiles.end()) {
        return 0;
    }
    
    std::string content = it->second;
    int count = 0;
    size_t pos = 0;
    
    // Count lines by finding newlines
    while ((pos = content.find('\n', pos)) != std::string::npos) {
        count++;
        pos++;
    }
    
    // If content doesn't end with newline, count the last line
    if (!content.empty() && content[content.length() - 1] != '\n') {
        count++;
    }
    
    return count;
}

bool FirmwareUpdater::extractVersionFromHex(const String& hexContent, String& version, String& buildDate) {
    // Reset version and buildDate
    version = "";
    buildDate = "";
    
    // Mock version extraction - look for common patterns
    if (hexContent.indexOf("VERSION:") != -1) {
        int start = hexContent.indexOf("VERSION:") + 8;
        int end = hexContent.indexOf("\n", start);
        if (end == -1) end = hexContent.length();
        version = hexContent.substring(start, end).trim();
    }
    
    if (hexContent.indexOf("BUILD:") != -1) {
        int start = hexContent.indexOf("BUILD:") + 6;
        int end = hexContent.indexOf("\n", start);
        if (end == -1) end = hexContent.length();
        buildDate = hexContent.substring(start, end).trim();
    }
    
    // Only return true if BOTH version and buildDate are found
    return version.length() > 0 && buildDate.length() > 0;
}

String FirmwareUpdater::parseHexLine(const String& hexLine) {
    if (isValidHexLine(hexLine)) {
        return hexLine.substring(1); // Remove leading ':'
    }
    return "";
}

bool FirmwareUpdater::isValidHexLine(const String& hexLine) {
    if (!hexLine.startsWith(":") || hexLine.length() < 11) {
        return false;
    }
    
    // Check if all characters after ':' are valid hex
    for (int i = 1; i < hexLine.length(); i++) {
        char c = hexLine.charAt(i);
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            return false;
        }
    }
    
    return true;
}

// .bin package handling methods
bool FirmwareUpdater::uploadFirmwarePackage(const uint8_t* packageData, size_t packageSize, const String& filename) {
    String filepath = getFirmwarePath(filename);
    
    Logger::addEntry("Uploading firmware package: " + filename);
    
    // Store in mock storage
    std::string data(reinterpret_cast<const char*>(packageData), packageSize);
    mockFirmwarePackages[filepath] = data;
    
    return true;
}

bool FirmwareUpdater::extractFirmwarePackage(const String& packagePath) {
    Logger::addEntry("Extracting firmware package: " + packagePath);
    return true; // Mock success
}

bool FirmwareUpdater::parseFirmwareMetadata(const String& metadataPath, String& version, String& description, String& buildDate, String& board) {
    // Mock metadata parsing
    version = "1.0.0";
    description = "Test Firmware";
    buildDate = "2024-01-15";
    board = "LC01";
    return true;
}

bool FirmwareUpdater::parseFirmwareMetadataFromString(const String& metadataJson, String& version, String& description, String& buildDate, String& board, String& features) {
    // Mock JSON parsing
    if (metadataJson.indexOf("\"version\"") != -1) {
        version = "1.0.0";
        description = "Test Firmware";
        buildDate = "2024-01-15";
        board = "LC01";
        features = "Feature 1, Feature 2";
        return true;
    }
    return false;
}

String FirmwareUpdater::getFirmwarePackageInfo(const String& filename) {
    if (!firmwarePackageExists(filename)) {
        return "Package not found";
    }
    
    String info = "Package: " + filename + "\n";
    info += "Size: " + String(std::to_string(getFirmwareSize(filename)).c_str()) + " bytes\n";
    info += "Type: Binary Package\n";
    
    return info;
}

bool FirmwareUpdater::deleteFirmwarePackage(const String& filename) {
    String filepath = getFirmwarePath(filename);
    
    if (mockFirmwarePackages.erase(filepath) > 0) {
        Logger::addEntry("Deleted firmware package: " + filename);
        return true;
    } else {
        Logger::addEntry("Failed to delete firmware package: " + filename);
        return false;
    }
}

String FirmwareUpdater::listFirmwarePackages() {
    String list = "Firmware Packages:\n";
    bool foundPackages = false;
    
    for (const auto& pair : mockFirmwarePackages) {
        String filename = pair.first;
        if (filename.startsWith("/")) {
            filename = filename.substring(1);
        }
        list += "- " + filename + " (" + std::to_string(pair.second.length()).c_str() + " bytes)\n";
        foundPackages = true;
    }
    
    if (!foundPackages) {
        list += "No packages found";
    }
    
    return list;
}

bool FirmwareUpdater::firmwarePackageExists(const String& filename) {
    return mockFirmwarePackages.find(getFirmwarePath(filename)) != mockFirmwarePackages.end();
}

String FirmwareUpdater::generateFirmwareFilename(const String& version, const String& board) {
    return "firmware-" + version + "-" + board + ".bin";
}

bool FirmwareUpdater::checkDuplicateFirmware(const String& version, const String& board) {
    String filename = generateFirmwareFilename(version, board);
    return firmwarePackageExists(filename);
}

String FirmwareUpdater::getAllFirmwareInfo() {
    String info = "=== Firmware Information ===\n\n";
    info += "Stored Firmwares:\n";
    info += listStoredFirmwares();
    info += "\n\nFirmware Packages:\n";
    info += listFirmwarePackages();
    return info;
}
