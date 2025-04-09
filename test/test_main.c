#include "unity.h"
#include <stdio.h>
#include "pico/stdio.h"
#include "pico/stdlib.h"

extern void test_flashmem(void);
extern void test_flashmem_multiple(void);
extern void test_flashmem_overwrite(void);

int main(void) {
    stdio_init_all();
    sleep_ms(2000);   // Give time for serial to connect
    printf("Begin unit tests..\n");

    UNITY_BEGIN();
    RUN_TEST(test_flashmem);
    RUN_TEST(test_flashmem_multiple);
    RUN_TEST(test_flashmem_overwrite);
    printf("Completed unit tests\n");
    return UNITY_END();
}
