#include "test_simple.h"
#include <string>

void test_basic_math(void) {
    TEST_ASSERT_EQUAL(4, 2 + 2);
    TEST_ASSERT_EQUAL(6, 2 * 3);
    TEST_ASSERT_EQUAL(2, 6 / 3);
    TEST_ASSERT_EQUAL(1, 5 % 2);
}

void test_string_operations(void) {
    std::string test = "Hello World";
    TEST_ASSERT_EQUAL(11, test.length());
    TEST_ASSERT_TRUE(test.find("Hello") != std::string::npos);
    TEST_ASSERT_TRUE(test.find("World") != std::string::npos);
}

void test_boolean_logic(void) {
    TEST_ASSERT_TRUE(true);
    TEST_ASSERT_FALSE(false);
    TEST_ASSERT_TRUE(1 == 1);
    TEST_ASSERT_FALSE(1 == 2);
}
