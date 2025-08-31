#include "test_logger_simple.h"
#include "mock_arduino.h"

// Simple test that doesn't depend on the actual Logger library
void test_logger_basic_functionality(void) {
    // Test our mock String class
    String testString = "Test log message";
    TEST_ASSERT_EQUAL(16, testString.length());
    TEST_ASSERT_EQUAL_STRING("Test log message", testString.c_str());
    
    // Test string concatenation
    String combined = "Hello " + String("World");
    TEST_ASSERT_EQUAL_STRING("Hello World", combined.c_str());
    
    // Test string comparison
    TEST_ASSERT_TRUE(testString == "Test log message");
    TEST_ASSERT_FALSE(testString == "Different message");
}

void test_logger_string_operations(void) {
    // Test string operations that the Logger would use
    String logEntry = "[1s] System started";
    
    // Test indexOf functionality
    TEST_ASSERT_EQUAL(0, logEntry.indexOf("[1s]"));
    TEST_ASSERT_EQUAL(5, logEntry.indexOf("System"));
    TEST_ASSERT_EQUAL(-1, logEntry.indexOf("Nonexistent"));
    
    // Test substring functionality
    String timestamp = logEntry.substring(0, 4);
    TEST_ASSERT_EQUAL_STRING("[1s]", timestamp.c_str());
    
    String message = logEntry.substring(5);
    TEST_ASSERT_EQUAL_STRING("System started", message.c_str());
}

void test_logger_edge_cases(void) {
    // Test edge cases that the Logger might encounter
    String emptyString = "";
    TEST_ASSERT_EQUAL(0, emptyString.length());
    TEST_ASSERT_EQUAL_STRING("", emptyString.c_str());
    
    // Test very long string
    String longString = "";
    for (int i = 0; i < 100; i++) {
        longString += "A";
    }
    TEST_ASSERT_EQUAL(100, longString.length());
    
    // Test string with special characters - Fixed: 30 chars, not 32
    String specialChars = "<script>alert('test')</script>";
    TEST_ASSERT_EQUAL(30, specialChars.length());
    TEST_ASSERT_TRUE(specialChars.indexOf("<script>") != -1);
    
    // Test string with numbers
    String numberString = "Log entry " + String(42);
    TEST_ASSERT_EQUAL_STRING("Log entry 42", numberString.c_str());
}
