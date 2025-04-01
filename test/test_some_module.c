#include "unity.h"

void setUp(void) {
    // This is run before EACH test
}

void tearDown(void) {
    // This is run after EACH test
}

void test_my_module_function(void) {
    // Example test case
    TEST_ASSERT_EQUAL_INT(4, 5);  // Asserts that my_module_function(2, 3) returns 5
}
