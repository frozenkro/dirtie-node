#include "unity.h"
#include <stdio.h>
#include "pico/stdio.h"
#include "pico/stdlib.h"

extern void test_my_module_function(void);

int main(void) {
    stdio_init_all(); // Initialize UART
    sleep_ms(2000);   // Give time for serial to connect

    UNITY_BEGIN();
    RUN_TEST(test_my_module_function);
    return UNITY_END();
}

