#include "test_logger.h"
#include "../lib/Logger/Logger.h"
#include "mock_arduino.h"

void test_logger_creation(void) {
    // Test Logger initialization
    Logger::init();
    
    // Test initial state
    TEST_ASSERT_EQUAL(0, Logger::getLogCount());
    TEST_ASSERT_EQUAL_STRING("", Logger::getLogs().c_str());
    
    // Test adding first log entry
    Logger::addEntry("Test log entry");
    TEST_ASSERT_EQUAL(1, Logger::getLogCount());
    
    // Test log entry content
    String logs = Logger::getLogs();
    TEST_ASSERT_TRUE(logs.indexOf("Test log entry") != -1);
    TEST_ASSERT_TRUE(logs.indexOf("log-entry") != -1);
}

void test_log_entry_formatting(void) {
    Logger::init();
    
    // Test basic log entry
    Logger::addEntry("Simple message");
    String logs = Logger::getLogs();
    
    // Verify HTML structure
    TEST_ASSERT_TRUE(logs.indexOf("<div class='log-entry'>") != -1);
    TEST_ASSERT_TRUE(logs.indexOf("Simple message") != -1);
    TEST_ASSERT_TRUE(logs.indexOf("</div>") != -1);
    
    // Test log entry with special characters
    Logger::addEntry("Message with <script>alert('xss')</script>");
    logs = Logger::getLogs();
    
    // Should escape HTML characters
    TEST_ASSERT_TRUE(logs.indexOf("&lt;script&gt;") != -1);
    TEST_ASSERT_TRUE(logs.indexOf("&lt;/script&gt;") != -1);
}

void test_log_html_generation(void) {
    Logger::init();
    
    // Test empty logger
    String logs = Logger::getLogs();
    TEST_ASSERT_EQUAL_STRING("", logs.c_str());
    
    // Test single log entry
    Logger::addEntry("Single entry");
    logs = Logger::getLogs();
    TEST_ASSERT_TRUE(logs.indexOf("Single entry") != -1);
    
    // Test multiple log entries
    Logger::addEntry("Second entry");
    Logger::addEntry("Third entry");
    logs = Logger::getLogs();
    
    // Should contain all entries
    TEST_ASSERT_TRUE(logs.indexOf("Single entry") != -1);
    TEST_ASSERT_TRUE(logs.indexOf("Second entry") != -1);
    TEST_ASSERT_TRUE(logs.indexOf("Third entry") != -1);
    
    // Count div elements (should be 3)
    int divCount = 0;
    int pos = 0;
    while ((pos = logs.indexOf("<div class='log-entry'>", pos)) != -1) {
        divCount++;
        pos++;
    }
    TEST_ASSERT_EQUAL(3, divCount);
}

void test_log_rotation(void) {
    Logger::init();
    
    // Test log rotation when exceeding max entries
    const int maxEntries = Logger::MAX_LOG_ENTRIES;
    
    // Add more than max entries
    for (int i = 0; i < maxEntries + 10; i++) {
        String message = "Log entry " + String(i);
        Logger::addEntry(message);
    }
    
    // Should maintain max entries
    TEST_ASSERT_EQUAL(maxEntries, Logger::getLogCount());
    
    // Should have newest entries
    String logs = Logger::getLogs();
    TEST_ASSERT_TRUE(logs.indexOf("Log entry 109") != -1); // Latest entry
    TEST_ASSERT_TRUE(logs.indexOf("Log entry 0") == -1);   // Oldest entry should be removed
}

void test_log_timestamp_parsing(void) {
    Logger::init();
    
    // Test timestamp format [Xs]
    Logger::addEntry("[1s] System started");
    Logger::addEntry("[2s] WiFi connected");
    Logger::addEntry("[10s] MQTT connected");
    
    String logs = Logger::getLogs();
    
    // Verify timestamps are preserved
    TEST_ASSERT_TRUE(logs.indexOf("[1s]") != -1);
    TEST_ASSERT_TRUE(logs.indexOf("[2s]") != -1);
    TEST_ASSERT_TRUE(logs.indexOf("[10s]") != -1);
    
    // Test mixed timestamp and non-timestamp entries
    Logger::addEntry("No timestamp entry");
    logs = Logger::getLogs();
    
    TEST_ASSERT_TRUE(logs.indexOf("No timestamp entry") != -1);
    TEST_ASSERT_TRUE(logs.indexOf("[1s]") != -1);
}

// Additional edge case tests for 100% coverage
void test_logger_edge_cases(void) {
    Logger::init();
    
    // Test empty string
    Logger::addEntry("");
    TEST_ASSERT_EQUAL(1, Logger::getLogCount());
    
    // Test very long message
    String longMessage = "";
    for (int i = 0; i < 1000; i++) {
        longMessage += "A";
    }
    Logger::addEntry(longMessage);
    TEST_ASSERT_EQUAL(2, Logger::getLogCount());
    
    // Test unicode characters
    Logger::addEntry("Unicode: 🚀✨🔥");
    TEST_ASSERT_EQUAL(3, Logger::getLogCount());
    
    // Test HTML injection attempts
    Logger::addEntry("<div onclick='alert(1)'>Click me</div>");
    String logs = Logger::getLogs();
    TEST_ASSERT_TRUE(logs.indexOf("&lt;div onclick=") != -1);
    
    // Test clear logs functionality
    Logger::clearLogs();
    TEST_ASSERT_EQUAL(0, Logger::getLogCount());
    TEST_ASSERT_EQUAL_STRING("", Logger::getLogs().c_str());
    
    // Test getLogEntries method
    Logger::addEntry("Plain text entry");
    String plainLogs = Logger::getLogEntries();
    TEST_ASSERT_TRUE(plainLogs.indexOf("Plain text entry") != -1);
}
