#include <unity.h>
#include "test_simple.h"
#include "test_logger_simple.h"
#include "test_logger_library.h"
#include "test_firmware_updater_library.h"

void setUp(void) {
    // Setup code that runs before each test
}

void tearDown(void) {
    // Cleanup code that runs after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Simple Tests - No library dependencies
    RUN_TEST(test_basic_math);
    RUN_TEST(test_string_operations);
    RUN_TEST(test_boolean_logic);
    
    // Logger Tests - Using mock Arduino system
    RUN_TEST(test_logger_basic_functionality);
    RUN_TEST(test_logger_string_operations);
    RUN_TEST(test_logger_edge_cases);
    
    // Logger Library Tests - Testing actual library functionality
    RUN_TEST(test_logger_init);
    RUN_TEST(test_logger_add_entry);
    RUN_TEST(test_logger_get_logs);
    RUN_TEST(test_logger_get_log_entries);
    RUN_TEST(test_logger_clear_logs);
    RUN_TEST(test_logger_get_log_count);
    RUN_TEST(test_logger_wrapping);
    RUN_TEST(test_logger_max_entries);
    
    // FirmwareUpdater Library Tests - Testing actual library functionality
    RUN_TEST(test_firmware_updater_init);
    RUN_TEST(test_firmware_updater_upload_to_spiffs);
    RUN_TEST(test_firmware_updater_firmware_exists);
    RUN_TEST(test_firmware_updater_get_firmware_size);
    RUN_TEST(test_firmware_updater_get_stored_firmware_info);
    RUN_TEST(test_firmware_updater_delete_stored_firmware);
    RUN_TEST(test_firmware_updater_list_stored_firmwares);
    RUN_TEST(test_firmware_updater_date_format_validation);
    RUN_TEST(test_firmware_updater_hex_line_validation);
    RUN_TEST(test_firmware_updater_parse_hex_line);
    RUN_TEST(test_firmware_updater_extract_version_from_hex);
    RUN_TEST(test_firmware_updater_count_hex_lines);
    RUN_TEST(test_firmware_updater_upload_firmware_package);
    RUN_TEST(test_firmware_updater_firmware_package_exists);
    RUN_TEST(test_firmware_updater_delete_firmware_package);
    RUN_TEST(test_firmware_updater_list_firmware_packages);
    RUN_TEST(test_firmware_updater_generate_firmware_filename);
    RUN_TEST(test_firmware_updater_check_duplicate_firmware);
    RUN_TEST(test_firmware_updater_get_all_firmware_info);
    
    // Real Firmware File Tests - Testing with actual firmware files
    RUN_TEST(test_real_firmware_files_exist);
    RUN_TEST(test_firmware_filename_generation);
    RUN_TEST(test_date_validation_logic);
    RUN_TEST(test_hex_validation_logic);
    RUN_TEST(test_firmware_info_formatting);
    
    // TODO: Add more library tests
    // ConfigManager Tests
    // WebHandler Tests
    // I2CScanner Tests
    // LEDController Tests
    
    return UNITY_END();
}
