#include "unity.h"
#include <stdio.h>
#include "pico/stdio.h"
#include "pico/stdlib.h"

extern void test_flashmem(void);

int main(void) {
    printf("Begin unit tests..\n");
    stdio_init_all();
    sleep_ms(2000);   // Give time for serial to connect

    UNITY_BEGIN();
    RUN_TEST(test_flashmem);
    printf("Completed unit tests\n");
    return UNITY_END();
}

