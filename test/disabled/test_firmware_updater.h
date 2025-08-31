#ifndef TEST_FIRMWARE_UPDATER_H
#define TEST_FIRMWARE_UPDATER_H

#include <unity.h>

// FirmwareUpdater Tests
void test_firmware_magic_header(void);
void test_firmware_metadata_parsing(void);
void test_firmware_hex_extraction(void);
void test_firmware_validation(void);
void test_firmware_error_handling(void);

#endif // TEST_FIRMWARE_UPDATER_H
