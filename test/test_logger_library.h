#ifndef TEST_LOGGER_LIBRARY_H
#define TEST_LOGGER_LIBRARY_H

#include <unity.h>

// Logger Library Tests - Testing actual library functionality
void test_logger_init(void);
void test_logger_add_entry(void);
void test_logger_get_logs(void);
void test_logger_get_log_entries(void);
void test_logger_clear_logs(void);
void test_logger_get_log_count(void);
void test_logger_wrapping(void);
void test_logger_max_entries(void);

#endif // TEST_LOGGER_LIBRARY_H
