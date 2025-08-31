#ifndef TEST_FIRMWAREUPDATER_LIBRARY_H
#define TEST_FIRMWAREUPDATER_LIBRARY_H

#include <unity.h>

// FirmwareUpdater Library Tests - Testing actual library functionality
void test_firmware_updater_init(void);
void test_firmware_updater_upload_to_spiffs(void);
void test_firmware_updater_firmware_exists(void);
void test_firmware_updater_get_firmware_size(void);
void test_firmware_updater_get_stored_firmware_info(void);
void test_firmware_updater_delete_stored_firmware(void);
void test_firmware_updater_list_stored_firmwares(void);
void test_firmware_updater_date_format_validation(void);
void test_firmware_updater_hex_line_validation(void);
void test_firmware_updater_parse_hex_line(void);
void test_firmware_updater_extract_version_from_hex(void);
void test_firmware_updater_count_hex_lines(void);
void test_firmware_updater_upload_firmware_package(void);
void test_firmware_updater_firmware_package_exists(void);
void test_firmware_updater_delete_firmware_package(void);
void test_firmware_updater_list_firmware_packages(void);
void test_firmware_updater_generate_firmware_filename(void);
void test_firmware_updater_check_duplicate_firmware(void);
void test_firmware_updater_get_all_firmware_info(void);

// Real Firmware File Tests - Testing with actual firmware files
void test_real_firmware_files_exist(void);
void test_firmware_filename_generation(void);
void test_date_validation_logic(void);
void test_hex_validation_logic(void);
void test_firmware_info_formatting(void);

#endif // TEST_FIRMWAREUPDATER_LIBRARY_H
