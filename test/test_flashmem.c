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

  char* res = malloc(MAX_VAL_SIZE);

  flashmem_err_t err = write(key, val);
  TEST_ASSERT_EQUAL(FM_ERR_OK, err);

  err = read(key, res);
  TEST_ASSERT_EQUAL(FM_ERR_OK, err);
  TEST_ASSERT_EQUAL(0, strcmp(val, res));

  free(res);
}

void test_flashmem_multiple() {
  const char* key1 = "TestKey1\0";
  const char* val1 = "TestVal1\0";
  const char* key2 = "TestKey2\0";
  const char* val2 = "TestVal2\0";
  const char* key3 = "TestKey3\0";
  const char* val3 = "TestVal3\0";

  char* res1 = malloc(MAX_VAL_SIZE);
  char* res2 = malloc(MAX_VAL_SIZE);
  char* res3 = malloc(MAX_VAL_SIZE);

  flashmem_err_t err = write(key1, val1);
  TEST_ASSERT_EQUAL(FM_ERR_OK, err);

  err = write(key2, val2);
  TEST_ASSERT_EQUAL(FM_ERR_OK, err);

  err = write(key3, val3);
  TEST_ASSERT_EQUAL(FM_ERR_OK, err);

  err = read(key1, res1);
  TEST_ASSERT_EQUAL(FM_ERR_OK, err);
  TEST_ASSERT_EQUAL(0, strcmp(val1, res1));

  err = read(key2, res2);
  TEST_ASSERT_EQUAL(FM_ERR_OK, err);
  TEST_ASSERT_EQUAL(0, strcmp(val2, res2));

  err = read(key3, res3);
  TEST_ASSERT_EQUAL(FM_ERR_OK, err);
  TEST_ASSERT_EQUAL(0, strcmp(val3, res3));

  free(res1);
  free(res2);
  free(res3);
}

void test_flashmem_overwrite() {
  const char* key = "TestKey\0";
  const char* val = "TestVal\0";
  const char* val2 = "TestVal2\0";

  char* res = malloc(MAX_VAL_SIZE);

  flashmem_err_t err = write(key, val);
  TEST_ASSERT_EQUAL(FM_ERR_OK, err);

  err = write(key, val2);
  TEST_ASSERT_EQUAL(FM_ERR_OK, err);

  err = read(key, res);
  TEST_ASSERT_EQUAL(FM_ERR_OK, err);
  TEST_ASSERT_EQUAL(0, strcmp(val2, res));

  free(res);
}

void test_flashmem_esc() {
  const char* key = "TestKey\0";
  const char* val = "TestVal\\,=\0";

  char* res = malloc(MAX_VAL_SIZE);

  flashmem_err_t err = write(key, val);
  TEST_ASSERT_EQUAL(FM_ERR_OK, err);

  err = read(key, res);
  TEST_ASSERT_EQUAL(FM_ERR_OK, err);
  TEST_ASSERT_EQUAL(0, strcmp(val, res));

  free(res);
}
