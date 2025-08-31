#include "test_logger_library.h"
#include "mock_arduino.h"
#include "mock_logger.h"

void test_logger_init(void) {
    // Test Logger initialization
    Logger::init();
    
    // After init, log count should be 0
    TEST_ASSERT_EQUAL(0, Logger::getLogCount());
    
    // Logs should be empty
    String logs = Logger::getLogs();
    TEST_ASSERT_TRUE(logs.indexOf("No log entries yet") != -1);
    
    String logEntries = Logger::getLogEntries();
    TEST_ASSERT_TRUE(logEntries.indexOf("No log entries yet") != -1);
}

void test_logger_add_entry(void) {
    // Clear logs first and reset state
    Logger::init();
    
    // Add a test entry
    Logger::addEntry("Test message");
    
    // Log count should be 1
    TEST_ASSERT_EQUAL(1, Logger::getLogCount());
    
    // Logs should contain our message
    String logs = Logger::getLogs();
    TEST_ASSERT_TRUE(logs.indexOf("Test message") != -1);
    
    // Should have timestamp format [Xs]
    TEST_ASSERT_TRUE(logs.indexOf("[1s]") != -1);
    
    // Log entries should also contain our message
    String logEntries = Logger::getLogEntries();
    TEST_ASSERT_TRUE(logEntries.indexOf("Test message") != -1);
}

void test_logger_get_logs(void) {
    // Clear logs first and reset state
    Logger::init();
    
    // Add multiple entries
    Logger::addEntry("First message");
    Logger::addEntry("Second message");
    Logger::addEntry("Third message");
    
    // Get HTML logs
    String logs = Logger::getLogs();
    
    // Should contain all messages
    TEST_ASSERT_TRUE(logs.indexOf("First message") != -1);
    TEST_ASSERT_TRUE(logs.indexOf("Second message") != -1);
    TEST_ASSERT_TRUE(logs.indexOf("Third message") != -1);
    
    // Should have HTML formatting
    TEST_ASSERT_TRUE(logs.indexOf("<div class='log-entry'>") != -1);
    
    // Log count should be 3
    TEST_ASSERT_EQUAL(3, Logger::getLogCount());
}

void test_logger_get_log_entries(void) {
    // Clear logs first and reset state
    Logger::init();
    
    // Add multiple entries
    Logger::addEntry("Plain text 1");
    Logger::addEntry("Plain text 2");
    
    // Get plain text log entries
    String logEntries = Logger::getLogEntries();
    
    // Should contain all messages
    TEST_ASSERT_TRUE(logEntries.indexOf("Plain text 1") != -1);
    TEST_ASSERT_TRUE(logEntries.indexOf("Plain text 2") != -1);
    
    // Should have newline separators
    TEST_ASSERT_TRUE(logEntries.indexOf("\n") != -1);
    
    // Should NOT have HTML formatting
    TEST_ASSERT_TRUE(logEntries.indexOf("<div") == -1);
}

void test_logger_clear_logs(void) {
    // Reset state first
    Logger::init();
    
    // Add some entries first
    Logger::addEntry("Message to be cleared");
    Logger::addEntry("Another message");
    
    // Verify we have entries
    TEST_ASSERT_EQUAL(2, Logger::getLogCount());
    
    // Clear logs
    Logger::clearLogs();
    
    // Log count should be 1 (clear adds an entry)
    TEST_ASSERT_EQUAL(1, Logger::getLogCount());
    
    // Should contain "Log cleared" message
    String logs = Logger::getLogs();
    TEST_ASSERT_TRUE(logs.indexOf("Log cleared") != -1);
}

void test_logger_get_log_count(void) {
    // Clear logs first and reset state
    Logger::init();
    
    // Initial count should be 0
    TEST_ASSERT_EQUAL(0, Logger::getLogCount());
    
    // Add entries and verify count
    for (int i = 0; i < 5; i++) {
        String message = "Message " + String(i);
        Logger::addEntry(message);
        TEST_ASSERT_EQUAL(i + 1, Logger::getLogCount());
    }
}

void test_logger_wrapping(void) {
    // Clear logs first and reset state
    Logger::init();
    
    // Add entries up to MAX_LOG_ENTRIES
    for (int i = 0; i < Logger::MAX_LOG_ENTRIES; i++) {
        String message = "Wrapping test " + String(i);
        Logger::addEntry(message);
    }
    
    // Log count should be MAX_LOG_ENTRIES
    TEST_ASSERT_EQUAL(Logger::MAX_LOG_ENTRIES, Logger::getLogCount());
    
    // Add one more entry to trigger wrapping
    Logger::addEntry("This should wrap");
    
    // Log count should still be MAX_LOG_ENTRIES
    TEST_ASSERT_EQUAL(Logger::MAX_LOG_ENTRIES, Logger::getLogCount());
    
    // Should contain the wrapped message
    String logs = Logger::getLogs();
    TEST_ASSERT_TRUE(logs.indexOf("This should wrap") != -1);
}

void test_logger_max_entries(void) {
    // Clear logs first and reset state
    Logger::init();
    
    // Verify MAX_LOG_ENTRIES constant
    TEST_ASSERT_EQUAL(100, Logger::MAX_LOG_ENTRIES);
    
    // Add exactly MAX_LOG_ENTRIES entries
    for (int i = 0; i < Logger::MAX_LOG_ENTRIES; i++) {
        String message = "Max test " + String(i);
        Logger::addEntry(message);
    }
    
    // Should have exactly MAX_LOG_ENTRIES
    TEST_ASSERT_EQUAL(Logger::MAX_LOG_ENTRIES, Logger::getLogCount());
    
    // Should contain first and last messages
    String logs = Logger::getLogs();
    TEST_ASSERT_TRUE(logs.indexOf("Max test 0") != -1);
    TEST_ASSERT_TRUE(logs.indexOf("Max test 99") != -1);
}
