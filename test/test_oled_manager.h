#ifndef TEST_OLED_MANAGER_H
#define TEST_OLED_MANAGER_H

// Test function declarations
void test_oled_init_success();
void test_oled_init_failure();
void test_oled_availability();
void test_oled_clear();
void test_oled_show_status();
void test_oled_show_system_info();
void test_oled_show_wifi_info();
void test_oled_show_i2c_info();
void test_oled_show_default_display();
void test_oled_update_display();
void test_uptime_calculation();
void test_wifi_status_connected();
void test_wifi_status_disconnected();
void test_i2c_communication_success();
void test_i2c_communication_failure();
void test_memory_management();

#endif
