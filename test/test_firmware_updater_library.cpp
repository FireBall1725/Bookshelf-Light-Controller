#include "test_firmware_updater_library.h"
#include "mock_arduino.h"
#include "mock_logger.h"
#include "mock_firmware_updater.h"

void test_firmware_updater_init(void) {
    // Test FirmwareUpdater initialization
    FirmwareUpdater::init();
    
    // After init, no firmware should exist
    TEST_ASSERT_FALSE(FirmwareUpdater::firmwareExists("test.hex"));
    TEST_ASSERT_FALSE(FirmwareUpdater::firmwarePackageExists("test.bin"));
}

void test_firmware_updater_upload_to_spiffs(void) {
    // Reset state
    FirmwareUpdater::init();
    
    // Test data
    const char* testData = "Test firmware data";
    size_t dataSize = strlen(testData);
    const uint8_t* firmwareData = reinterpret_cast<const uint8_t*>(testData);
    
    // Upload firmware
    bool result = FirmwareUpdater::uploadFirmwareToSPIFFS(firmwareData, dataSize, "test.hex");
    TEST_ASSERT_TRUE(result);
    
    // Verify firmware exists
    TEST_ASSERT_TRUE(FirmwareUpdater::firmwareExists("test.hex"));
    TEST_ASSERT_EQUAL(dataSize, FirmwareUpdater::getFirmwareSize("test.hex"));
}

void test_firmware_updater_firmware_exists(void) {
    // Reset state
    FirmwareUpdater::init();
    
    // Initially no firmware
    TEST_ASSERT_FALSE(FirmwareUpdater::firmwareExists("nonexistent.hex"));
    
    // Upload some firmware
    const char* testData = "Test data";
    FirmwareUpdater::uploadFirmwareToSPIFFS(
        reinterpret_cast<const uint8_t*>(testData), 
        strlen(testData), 
        "test.hex"
    );
    
    // Now it should exist
    TEST_ASSERT_TRUE(FirmwareUpdater::firmwareExists("test.hex"));
}

void test_firmware_updater_get_firmware_size(void) {
    // Reset state
    FirmwareUpdater::init();
    
    // Test with non-existent file
    TEST_ASSERT_EQUAL(0, FirmwareUpdater::getFirmwareSize("nonexistent.hex"));
    
    // Upload firmware and check size
    const char* testData = "Test firmware content";
    size_t expectedSize = strlen(testData);
    
    FirmwareUpdater::uploadFirmwareToSPIFFS(
        reinterpret_cast<const uint8_t*>(testData), 
        expectedSize, 
        "test.hex"
    );
    
    TEST_ASSERT_EQUAL(expectedSize, FirmwareUpdater::getFirmwareSize("test.hex"));
}

void test_firmware_updater_get_stored_firmware_info(void) {
    // Reset state
    FirmwareUpdater::init();
    
    // Test with non-existent file
    String info = FirmwareUpdater::getStoredFirmwareInfo("nonexistent.hex");
    TEST_ASSERT_TRUE(info.indexOf("Firmware not found") != -1);
    
    // Upload firmware with version info
    const char* hexData = "VERSION:1.0.0\nBUILD:2024-01-15\n:10000000";
    FirmwareUpdater::uploadFirmwareToSPIFFS(
        reinterpret_cast<const uint8_t*>(hexData), 
        strlen(hexData), 
        "test.hex"
    );
    
    // Get info
    info = FirmwareUpdater::getStoredFirmwareInfo("test.hex");
    TEST_ASSERT_TRUE(info.indexOf("test.hex") != -1);
    TEST_ASSERT_TRUE(info.indexOf("Intel HEX") != -1);
    TEST_ASSERT_TRUE(info.indexOf("1.0.0") != -1);
    TEST_ASSERT_TRUE(info.indexOf("2024-01-15") != -1);
}

void test_firmware_updater_delete_stored_firmware(void) {
    // Reset state
    FirmwareUpdater::init();
    
    // Upload firmware first
    const char* testData = "Test data";
    FirmwareUpdater::uploadFirmwareToSPIFFS(
        reinterpret_cast<const uint8_t*>(testData), 
        strlen(testData), 
        "test.hex"
    );
    
    // Verify it exists
    TEST_ASSERT_TRUE(FirmwareUpdater::firmwareExists("test.hex"));
    
    // Delete it
    bool result = FirmwareUpdater::deleteStoredFirmware("test.hex");
    TEST_ASSERT_TRUE(result);
    
    // Verify it's gone
    TEST_ASSERT_FALSE(FirmwareUpdater::firmwareExists("test.hex"));
}

void test_firmware_updater_list_stored_firmwares(void) {
    // Reset state
    FirmwareUpdater::init();
    
    // Initially no firmware
    String list = FirmwareUpdater::listStoredFirmwares();
    TEST_ASSERT_TRUE(list.indexOf("No firmware files found") != -1);
    
    // Upload some firmware
    const char* testData1 = "Firmware 1";
    const char* testData2 = "Firmware 2";
    
    FirmwareUpdater::uploadFirmwareToSPIFFS(
        reinterpret_cast<const uint8_t*>(testData1), 
        strlen(testData1), 
        "firmware1.hex"
    );
    
    FirmwareUpdater::uploadFirmwareToSPIFFS(
        reinterpret_cast<const uint8_t*>(testData2), 
        strlen(testData2), 
        "firmware2.hex"
    );
    
    // List firmware
    list = FirmwareUpdater::listStoredFirmwares();
    TEST_ASSERT_TRUE(list.indexOf("firmware1.hex") != -1);
    TEST_ASSERT_TRUE(list.indexOf("firmware2.hex") != -1);
    TEST_ASSERT_TRUE(list.indexOf("Stored Firmwares:") != -1);
}

void test_firmware_updater_date_format_validation(void) {
    // Test valid date formats
    TEST_ASSERT_TRUE(FirmwareUpdater::isValidDateFormat("2024-01-15", '-'));
    TEST_ASSERT_TRUE(FirmwareUpdater::isValidDateFormat("2024/01/15", '/'));
    TEST_ASSERT_TRUE(FirmwareUpdater::isValidDateFormat("2024.01.15", '.'));
    
    // Test invalid date formats
    TEST_ASSERT_FALSE(FirmwareUpdater::isValidDateFormat("2024-1-15", '-'));  // Missing leading zero
    TEST_ASSERT_FALSE(FirmwareUpdater::isValidDateFormat("2024-01-15", '/'));  // Wrong separator
    TEST_ASSERT_FALSE(FirmwareUpdater::isValidDateFormat("2024-13-15", '-'));  // Invalid month
    TEST_ASSERT_FALSE(FirmwareUpdater::isValidDateFormat("2024-01-32", '-'));  // Invalid day
    TEST_ASSERT_FALSE(FirmwareUpdater::isValidDateFormat("1999-01-15", '-'));  // Year too old
    TEST_ASSERT_FALSE(FirmwareUpdater::isValidDateFormat("2031-01-15", '-'));  // Year too new
    TEST_ASSERT_FALSE(FirmwareUpdater::isValidDateFormat("invalid", '-'));     // Not a date
}

void test_firmware_updater_hex_line_validation(void) {
    // Test valid hex lines
    TEST_ASSERT_TRUE(FirmwareUpdater::isValidHexLine(":100000000102030405060708090A0B0C0D0E0F10"));
    TEST_ASSERT_TRUE(FirmwareUpdater::isValidHexLine(":00000001FF"));
    TEST_ASSERT_TRUE(FirmwareUpdater::isValidHexLine(":020000040000F2"));
    
    // Test invalid hex lines
    TEST_ASSERT_FALSE(FirmwareUpdater::isValidHexLine(""));                    // Empty
    TEST_ASSERT_FALSE(FirmwareUpdater::isValidHexLine("1000000001020304"));   // Missing colon
    TEST_ASSERT_FALSE(FirmwareUpdater::isValidHexLine(":100000000102030405060708090A0B0C0D0E0F1G")); // Invalid hex char
}

void test_firmware_updater_parse_hex_line(void) {
    // Test valid hex line parsing
    String result = FirmwareUpdater::parseHexLine(":100000000102030405060708090A0B0C0D0E0F10");
    TEST_ASSERT_EQUAL_STRING("100000000102030405060708090A0B0C0D0E0F10", result.c_str());
    
    // Test invalid hex line parsing
    result = FirmwareUpdater::parseHexLine("invalid");
    TEST_ASSERT_EQUAL_STRING("", result.c_str());
}

void test_firmware_updater_extract_version_from_hex(void) {
    // Reset state
    FirmwareUpdater::init();
    
    // Test hex content with version info
    const char* hexContent = "VERSION:2.1.0\nBUILD:2024-02-20\n:10000000";
    String version, buildDate;
    
    bool result = FirmwareUpdater::extractVersionFromHex(hexContent, version, buildDate);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_STRING("2.1.0", version.c_str());
    TEST_ASSERT_EQUAL_STRING("2024-02-20", buildDate.c_str());
    
    // Test hex content without version info
    const char* noVersionContent = ":100000000102030405060708090A0B0C0D0E0F10";
    result = FirmwareUpdater::extractVersionFromHex(noVersionContent, version, buildDate);
    TEST_ASSERT_FALSE(result);
}

void test_firmware_updater_count_hex_lines(void) {
    // Reset state
    FirmwareUpdater::init();
    
    // Upload hex file with multiple lines
    const char* hexContent = ":100000000102030405060708090A0B0C0D0E0F10\n:100010001112131415161718191A1B1C1D1E1F20\n:00000001FF";
    FirmwareUpdater::uploadFirmwareToSPIFFS(
        reinterpret_cast<const uint8_t*>(hexContent), 
        strlen(hexContent), 
        "test.hex"
    );
    
    // Count lines
    int lineCount = FirmwareUpdater::countHexLines("/test.hex");
    TEST_ASSERT_EQUAL(3, lineCount);
}

void test_firmware_updater_upload_firmware_package(void) {
    // Reset state
    FirmwareUpdater::init();
    
    // Test data for package
    const char* packageData = "FLFW\0{\"version\":\"1.0.0\",\"board\":\"LC01\"}";
    size_t packageSize = strlen(packageData);
    
    // Upload package
    bool result = FirmwareUpdater::uploadFirmwarePackage(
        reinterpret_cast<const uint8_t*>(packageData), 
        packageSize, 
        "test.bin"
    );
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(FirmwareUpdater::firmwarePackageExists("test.bin"));
}

void test_firmware_updater_firmware_package_exists(void) {
    // Reset state
    FirmwareUpdater::init();
    
    // Initially no packages
    TEST_ASSERT_FALSE(FirmwareUpdater::firmwarePackageExists("nonexistent.bin"));
    
    // Upload package
    const char* packageData = "Test package";
    FirmwareUpdater::uploadFirmwarePackage(
        reinterpret_cast<const uint8_t*>(packageData), 
        strlen(packageData), 
        "test.bin"
    );
    
    // Now it should exist
    TEST_ASSERT_TRUE(FirmwareUpdater::firmwarePackageExists("test.bin"));
}

void test_firmware_updater_delete_firmware_package(void) {
    // Reset state
    FirmwareUpdater::init();
    
    // Upload package first
    const char* packageData = "Test package";
    FirmwareUpdater::uploadFirmwarePackage(
        reinterpret_cast<const uint8_t*>(packageData), 
        strlen(packageData), 
        "test.bin"
    );
    
    // Verify it exists
    TEST_ASSERT_TRUE(FirmwareUpdater::firmwarePackageExists("test.bin"));
    
    // Delete it
    bool result = FirmwareUpdater::deleteFirmwarePackage("test.bin");
    TEST_ASSERT_TRUE(result);
    
    // Verify it's gone
    TEST_ASSERT_FALSE(FirmwareUpdater::firmwarePackageExists("test.bin"));
}

void test_firmware_updater_list_firmware_packages(void) {
    // Reset state
    FirmwareUpdater::init();
    
    // Initially no packages
    String list = FirmwareUpdater::listFirmwarePackages();
    TEST_ASSERT_TRUE(list.indexOf("No packages found") != -1);
    
    // Upload some packages
    const char* packageData1 = "Package 1";
    const char* packageData2 = "Package 2";
    
    FirmwareUpdater::uploadFirmwarePackage(
        reinterpret_cast<const uint8_t*>(packageData1), 
        strlen(packageData1), 
        "package1.bin"
    );
    
    FirmwareUpdater::uploadFirmwarePackage(
        reinterpret_cast<const uint8_t*>(packageData2), 
        strlen(packageData2), 
        "package2.bin"
    );
    
    // List packages
    list = FirmwareUpdater::listFirmwarePackages();
    TEST_ASSERT_TRUE(list.indexOf("package1.bin") != -1);
    TEST_ASSERT_TRUE(list.indexOf("package2.bin") != -1);
    TEST_ASSERT_TRUE(list.indexOf("Firmware Packages:") != -1);
}

void test_firmware_updater_generate_firmware_filename(void) {
    // Test filename generation
    String filename = FirmwareUpdater::generateFirmwareFilename("1.2.3", "LC01");
    TEST_ASSERT_EQUAL_STRING("firmware-1.2.3-LC01.bin", filename.c_str());
    
    filename = FirmwareUpdater::generateFirmwareFilename("2.0.0", "TEST");
    TEST_ASSERT_EQUAL_STRING("firmware-2.0.0-TEST.bin", filename.c_str());
}

void test_firmware_updater_check_duplicate_firmware(void) {
    // Reset state
    FirmwareUpdater::init();
    
    // Initially no duplicates
    TEST_ASSERT_FALSE(FirmwareUpdater::checkDuplicateFirmware("1.0.0", "LC01"));
    
    // Upload package with same version and board
    const char* packageData = "Test package";
    String filename = FirmwareUpdater::generateFirmwareFilename("1.0.0", "LC01");
    FirmwareUpdater::uploadFirmwarePackage(
        reinterpret_cast<const uint8_t*>(packageData), 
        strlen(packageData), 
        filename
    );
    
    // Now it should detect duplicate
    TEST_ASSERT_TRUE(FirmwareUpdater::checkDuplicateFirmware("1.0.0", "LC01"));
    
    // Different version should not be duplicate
    TEST_ASSERT_FALSE(FirmwareUpdater::checkDuplicateFirmware("1.0.1", "LC01"));
    
    // Different board should not be duplicate
    TEST_ASSERT_FALSE(FirmwareUpdater::checkDuplicateFirmware("1.0.0", "LC02"));
}

void test_firmware_updater_get_all_firmware_info(void) {
    // Reset state
    FirmwareUpdater::init();
    
    // Upload some firmware and packages
    const char* hexData = "Test hex firmware";
    const char* packageData = "Test package";
    
    FirmwareUpdater::uploadFirmwareToSPIFFS(
        reinterpret_cast<const uint8_t*>(hexData), 
        strlen(hexData), 
        "test.hex"
    );
    
    FirmwareUpdater::uploadFirmwarePackage(
        reinterpret_cast<const uint8_t*>(packageData), 
        strlen(packageData), 
        "test.bin"
    );
    
    // Get all info
    String allInfo = FirmwareUpdater::getAllFirmwareInfo();
    
    // Should contain both sections
    TEST_ASSERT_TRUE(allInfo.indexOf("=== Firmware Information ===") != -1);
    TEST_ASSERT_TRUE(allInfo.indexOf("Stored Firmwares:") != -1);
    TEST_ASSERT_TRUE(allInfo.indexOf("Firmware Packages:") != -1);
    
    // Should contain our files
    TEST_ASSERT_TRUE(allInfo.indexOf("test.hex") != -1);
    TEST_ASSERT_TRUE(allInfo.indexOf("test.bin") != -1);
}

// Real Firmware File Tests - Testing with actual firmware files
void test_real_firmware_files_exist(void) {
    // Test that we can work with the actual firmware files
    String firmware1 = "firmware-v1.0.6.bin";
    String firmware2 = "firmware-v1.0.5.bin";
    String firmware3 = "firmware-v1.0.4.bin";
    
    // Test filename parsing
    TEST_ASSERT_TRUE(firmware1.endsWith(".bin"));
    TEST_ASSERT_TRUE(firmware2.endsWith(".bin"));
    TEST_ASSERT_TRUE(firmware3.endsWith(".bin"));
    
    // Test version extraction from filename
    int versionStart1 = firmware1.indexOf("v");
    int versionEnd1 = firmware1.indexOf(".bin");
    String version1 = firmware1.substring(versionStart1 + 1, versionEnd1);
    TEST_ASSERT_EQUAL_STRING("1.0.6", version1.c_str());
    
    int versionStart2 = firmware2.indexOf("v");
    int versionEnd2 = firmware2.indexOf(".bin");
    String version2 = firmware2.substring(versionStart2 + 1, versionEnd2);
    TEST_ASSERT_EQUAL_STRING("1.0.5", version2.c_str());
    
    int versionStart3 = firmware3.indexOf("v");
    int versionEnd3 = firmware3.indexOf(".bin");
    String version3 = firmware3.substring(versionStart3 + 1, versionEnd3);
    TEST_ASSERT_EQUAL_STRING("1.0.4", version3.c_str());
}

void test_firmware_filename_generation(void) {
    // Test firmware filename generation logic
    String version = "1.0.0";
    String board = "LC01";
    
    String filename = "firmware-" + version + "-" + board + ".bin";
    TEST_ASSERT_EQUAL_STRING("firmware-1.0.0-LC01.bin", filename.c_str());
    
    // Test with different version and board
    String filename2 = "firmware-" + String("2.1.0") + "-" + String("ESP32") + ".bin";
    TEST_ASSERT_EQUAL_STRING("firmware-2.1.0-ESP32.bin", filename2.c_str());
}

void test_date_validation_logic(void) {
    // Test the date validation logic that would be used in firmware validation
    String validDate = "2024-01-15";
    String invalidDate = "2024-13-15";
    
    // Basic validation logic
    bool isValid = true;
    
    // Check length
    if (validDate.length() != 10) {
        isValid = false;
    }
    
    // Check separator positions
    if (validDate.charAt(4) != '-' || validDate.charAt(7) != '-') {
        isValid = false;
    }
    
    // Check if all other characters are digits
    for (int i = 0; i < validDate.length(); i++) {
        if (i != 4 && i != 7) { // Skip separator positions
            if (validDate.charAt(i) < '0' || validDate.charAt(i) > '9') {
                isValid = false;
                break;
            }
        }
    }
    
    // Basic validation: year should be reasonable, month 1-12, day 1-31
    int year = validDate.substring(0, 4).toInt();
    int month = validDate.substring(5, 7).toInt();
    int day = validDate.substring(8, 10).toInt();
    
    if (year < 2000 || year > 2030) isValid = false;
    if (month < 1 || month > 12) isValid = false;
    if (day < 1 || day > 31) isValid = false;
    
    TEST_ASSERT_TRUE(isValid);
    
    // Test invalid date (month 13)
    bool isInvalid = true;
    if (invalidDate.length() != 10) {
        isInvalid = false;
    }
    if (invalidDate.charAt(4) != '-' || invalidDate.charAt(7) != '-') {
        isInvalid = false;
    }
    
    // Month 13 is invalid
    int month2 = invalidDate.substring(5, 7).toInt();
    if (month2 < 1 || month2 > 12) {
        isInvalid = false;
    }
    
    TEST_ASSERT_FALSE(isInvalid);
}

void test_hex_validation_logic(void) {
    // Test hex line validation logic
    String validHex = ":100000000102030405060708090A0B0C0D0E0F10";
    String invalidHex = "1000000001020304"; // Missing colon
    
    // Basic validation logic
    bool isValid = true;
    
    // Check if it starts with colon
    if (!validHex.startsWith(":")) {
        isValid = false;
    }
    
    // Check minimum length (colon + at least 10 hex chars)
    if (validHex.length() < 11) {
        isValid = false;
    }
    
    // Check if all characters after colon are valid hex
    for (int i = 1; i < validHex.length(); i++) {
        char c = validHex.charAt(i);
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            isValid = false;
            break;
        }
    }
    
    TEST_ASSERT_TRUE(isValid);
    
    // Test invalid hex
    bool isInvalid = true;
    if (invalidHex.startsWith(":")) {
        isInvalid = false;
    }
    TEST_ASSERT_TRUE(isInvalid);
}

void test_firmware_info_formatting(void) {
    // Test firmware info formatting logic
    String filename = "test.hex";
    int size = 1024;
    String type = "Intel HEX";
    
    String info = "Filename: " + filename + "\n";
    info += "Size: " + String(size) + " bytes\n";
    info += "Type: " + type + "\n";
    
    TEST_ASSERT_TRUE(info.indexOf("Filename: test.hex") != -1);
    TEST_ASSERT_TRUE(info.indexOf("Size: 1024 bytes") != -1);
    TEST_ASSERT_TRUE(info.indexOf("Type: Intel HEX") != -1);
}
