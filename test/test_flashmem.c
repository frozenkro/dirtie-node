#include "unity.h"
#include "flashmem/flashmem.h"
#include <stdlib.h>
#include <string.h>

void setUp(void) {
    // This is run before EACH test
}

void tearDown(void) {
    // This is run after EACH test
}

void test_flashmem() {
  const char* key = "TestKey\0";
  const char* val = "TestVal\0";

  char* res = malloc(strlen(val));

  flashmem_err_t err = write(key, val);
  TEST_ASSERT_EQUAL(FM_ERR_OK, err);

  err = read(key, res);
  TEST_ASSERT_EQUAL(FM_ERR_OK, err);
  TEST_ASSERT_EQUAL(0, strcmp(val, res));

  free(res);
}
