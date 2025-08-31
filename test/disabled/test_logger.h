#ifndef TEST_LOGGER_H
#define TEST_LOGGER_H

#include <unity.h>

// Logger Tests
void test_logger_creation(void);
void test_log_entry_formatting(void);
void test_log_html_generation(void);
void test_log_rotation(void);
void test_log_timestamp_parsing(void);

#endif // TEST_LOGGER_H
