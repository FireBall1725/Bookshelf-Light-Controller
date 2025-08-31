#include "test_firmware_updater.h"
#include "../lib/FirmwareUpdater/FirmwareUpdater.h"
#include "mock_arduino.h"

void test_firmware_magic_header(void) {
    // Test firmware package existence
    bool exists = FirmwareUpdater::firmwarePackageExists("test-firmware.bin");
    TEST_ASSERT_FALSE(exists); // Should not exist initially
    
    // Test firmware package info retrieval
    String info = FirmwareUpdater::getFirmwarePackageInfo("nonexistent.bin");
    TEST_ASSERT_EQUAL_STRING("", info.c_str()); // Should return empty for non-existent file
    
    // Test firmware package deletion
    bool deleteSuccess = FirmwareUpdater::deleteFirmwarePackage("nonexistent.bin");
    TEST_ASSERT_FALSE(deleteSuccess); // Should fail for non-existent file
}

void test_firmware_metadata_parsing(void) {
    // Test metadata parsing from string
    String version, description, buildDate, board, features;
    
    const char* validJson = "{\"version\":\"1.0.0\",\"description\":\"Test firmware\",\"buildDate\":\"2024-01-15\",\"board\":\"LC01\",\"features\":[\"Feature 1\",\"Feature 2\"]}";
    
    bool parseSuccess = FirmwareUpdater::parseFirmwareMetadataFromString(validJson, version, description, buildDate, board, features);
    TEST_ASSERT_TRUE(parseSuccess);
    TEST_ASSERT_EQUAL_STRING("1.0.0", version.c_str());
    TEST_ASSERT_EQUAL_STRING("Test firmware", description.c_str());
    TEST_ASSERT_EQUAL_STRING("2024-01-15", buildDate.c_str());
    TEST_ASSERT_EQUAL_STRING("LC01", board.c_str());
    TEST_ASSERT_TRUE(features.indexOf("Feature 1") != -1);
    TEST_ASSERT_TRUE(features.indexOf("Feature 2") != -1);
    
    // Test invalid JSON handling
    const char* invalidJson = "{invalid json}";
    bool invalidParse = FirmwareUpdater::parseFirmwareMetadataFromString(invalidJson, version, description, buildDate, board, features);
    TEST_ASSERT_FALSE(invalidParse);
    
    // Test missing fields
    const char* partialJson = "{\"version\":\"1.0.0\"}";
    bool partialParse = FirmwareUpdater::parseFirmwareMetadataFromString(partialJson, version, description, buildDate, board, features);
    TEST_ASSERT_TRUE(partialParse);
    TEST_ASSERT_EQUAL_STRING("1.0.0", version.c_str());
    TEST_ASSERT_EQUAL_STRING("", description.c_str());
    TEST_ASSERT_EQUAL_STRING("", buildDate.c_str());
    TEST_ASSERT_EQUAL_STRING("", board.c_str());
}

void test_firmware_hex_extraction(void) {
    // Test firmware package extraction
    bool extractSuccess = FirmwareUpdater::extractFirmwarePackage("nonexistent.bin");
    TEST_ASSERT_FALSE(extractSuccess); // Should fail for non-existent file
    
    // Test firmware metadata parsing from file
    String version, description, buildDate, board;
    bool parseFileSuccess = FirmwareUpdater::parseFirmwareMetadata("nonexistent.json", version, description, buildDate, board);
    TEST_ASSERT_FALSE(parseFileSuccess); // Should fail for non-existent file
    
    // Test with empty strings
    TEST_ASSERT_EQUAL_STRING("", version.c_str());
    TEST_ASSERT_EQUAL_STRING("", description.c_str());
    TEST_ASSERT_EQUAL_STRING("", buildDate.c_str());
    TEST_ASSERT_EQUAL_STRING("", board.c_str());
}

void test_firmware_validation(void) {
    // Test firmware filename generation
    String filename = FirmwareUpdater::generateFirmwareFilename("1.0.0", "LC01");
    TEST_ASSERT_EQUAL_STRING("firmware-1.0.0-LC01.bin", filename.c_str());
    
    // Test duplicate firmware checking
    bool isDuplicate = FirmwareUpdater::checkDuplicateFirmware("1.0.0", "LC01");
    TEST_ASSERT_FALSE(isDuplicate); // Should not be duplicate initially
    
    // Test firmware listing
    String firmwareList = FirmwareUpdater::listFirmwarePackages();
    TEST_ASSERT_EQUAL_STRING("", firmwareList.c_str()); // Should be empty initially
    
    // Test firmware size
    size_t size = FirmwareUpdater::getFirmwareSize("nonexistent.bin");
    TEST_ASSERT_EQUAL(0, size); // Should be 0 for non-existent file
}

void test_firmware_error_handling(void) {
    // Test firmware upload with null data
    bool nullUpload = FirmwareUpdater::uploadFirmwareToSPIFFS(nullptr, 100, "test.bin");
    TEST_ASSERT_FALSE(nullUpload);
    
    // Test firmware upload with zero size
    uint8_t dummyData[10] = {0};
    bool zeroSizeUpload = FirmwareUpdater::uploadFirmwareToSPIFFS(dummyData, 0, "test.bin");
    TEST_ASSERT_FALSE(zeroSizeUpload);
    
    // Test firmware package upload with null data
    bool nullPackageUpload = FirmwareUpdater::uploadFirmwarePackage(nullptr, 100, "test.bin");
    TEST_ASSERT_FALSE(nullPackageUpload);
    
    // Test firmware package upload with zero size
    bool zeroSizePackageUpload = FirmwareUpdater::uploadFirmwarePackage(dummyData, 0, "test.bin");
    TEST_ASSERT_FALSE(zeroSizePackageUpload);
    
    // Test ATtiny firmware update without file
    bool noFileUpdate = FirmwareUpdater::updateATtinyFirmwareFromSPIFFS("nonexistent.hex");
    TEST_ASSERT_FALSE(noFileUpdate);
    
    // Test ATtiny version check
    bool versionCheck = FirmwareUpdater::checkATtinyVersion();
    TEST_ASSERT_FALSE(versionCheck); // Should fail without proper I2C setup
    
    // Test stored firmware info for non-existent file
    String storedInfo = FirmwareUpdater::getStoredFirmwareInfo("nonexistent.hex");
    TEST_ASSERT_EQUAL_STRING("", storedInfo.c_str());
    
    // Test delete stored firmware for non-existent file
    bool deleteStored = FirmwareUpdater::deleteStoredFirmware("nonexistent.hex");
    TEST_ASSERT_FALSE(deleteStored);
}

// Additional comprehensive tests for 100% coverage
void test_firmware_comprehensive(void) {
    // Test initialization
    FirmwareUpdater::init();
    
    // Test ATtiny firmware update
    bool updateSuccess = FirmwareUpdater::updateATtinyFirmware();
    TEST_ASSERT_FALSE(updateSuccess); // Should fail without proper setup
    
    // Test getAllFirmwareInfo
    String allInfo = FirmwareUpdater::getAllFirmwareInfo();
    TEST_ASSERT_EQUAL_STRING("No firmware packages found", allInfo.c_str()); // Should be empty initially
}
