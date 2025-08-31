#ifndef TEST_CONFIG_MANAGER_H
#define TEST_CONFIG_MANAGER_H

#include <unity.h>

// ConfigManager Tests
void test_wifi_config_storage(void);
void test_mqtt_config_storage(void);
void test_config_validation(void);
void test_config_defaults(void);
void test_config_persistence(void);

#endif // TEST_CONFIG_MANAGER_H
