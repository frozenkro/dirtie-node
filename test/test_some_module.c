// test/test_some_module.c
#include "unity.h"

void setUp(void) {
    // This is run before EACH test
}

void tearDown(void) {
    // This is run after EACH test
}

void test_my_module_function(void) {
    // Example test case
    TEST_ASSERT_EQUAL_INT(5, 5);  // Asserts that my_module_function(2, 3) returns 5
}

/* int main(void) { */
/*     UNITY_BEGIN(); */
/*     RUN_TEST(test_my_module_function);  // Add all your test functions here */
/*     return UNITY_END(); */
/* } */

