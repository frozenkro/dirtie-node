#include "pico/stdlib.h"
#include "stdlib.h"
#include <lwip/netif.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "hardware/flash.h"

// Defined in memmap_custom.ld
extern uint32_t ADDR_PERSISTENT[];
#define ADDR_PERSISTENT_BASE (ADDR_PERSISTENT)

// Can only write flash in pages of 256 bytes
#define PAGE_SIZE 256
// flash sector alignment size
#define NVS_SIZE 4096

enum _flashmem_err 
{
  FM_ERR_OK = 0,
  FM_ERR_FLASHMEM_SIZE = 1,
  FM_ERR_INVALID_KEY = 2,
};
typedef enum _flashmem_err flashmem_err_t;

// key1=val1,key2=val2,(etc..)\0

char* esc_chars(char* val) {
  if (val == NULL) {
    return NULL;
  }

  size_t special_ch_ct = 0;
  size_t val_len = 0;
  for (const char* c = val; *c != '\0'; c++) {
    val_len++;
    if (*c == '=' || *c == ',' || *c == '\\') {
      special_ch_ct++;
    }
  }

  char* res = malloc(val_len + special_ch_ct + 1);
  if (res == NULL) {
    return NULL;
  }

  char* head = res;
  for (const char* src = val; *src != '\0'; src++) {
    if (*src == '=' || *src == ',' || *src == '\\') {
      *head++ = '\\';
    }
    *head = *src;
  }
  *head = '\0';

  return res;
}

flashmem_err_t upsert_key(char* key, char* val, char* buf) {
  if (strrchr(key, '\\') != NULL || strrchr(key, ',') != NULL || strrchr(key, '=') != NULL) {
    return FM_ERR_INVALID_KEY;
  }

  // TODO escape all invalid chars in val
  if (strlen(key) + strlen(val) + strlen(buf) > NVS_SIZE - 1) {
    return FM_ERR_FLASHMEM_SIZE;
  }

  int keylen = strlen(key);
  char *key_search = malloc(keylen + 2);
  strcpy(key_search, key);
  key_search[keylen + 1] = '=';
  key_search[keylen + 2] = '\0';

  // if key not present, just append
  if (strstr(buf, key_search) == NULL) {
    char *end = strrchr(buf, '\0');
    strcpy(end, key_search);
    end = strrchr(buf, '\0');
    strcpy(end, val);
    // does strcpy add null to the end of dest?
  }
  else {

  }
  // else splice new value

  free(key_search);
  return FM_ERR_OK;
}

int write(char* key, char* val) {
  uint8_t *buf = malloc(NVS_SIZE);

  memcpy(buf, ADDR_PERSISTENT, NVS_SIZE);

  if (upsert_key(key, val, (char*)buf)) {

  }

  unsigned int mem_size = strlen((char*)buf);
  int write_size = (mem_size / 256) + (256 - (mem_size % 256));

  //START Critical Section
  flash_range_erase((uint32_t)ADDR_PERSISTENT_BASE - XIP_BASE, NVS_SIZE);
  flash_range_program((uint32_t)ADDR_PERSISTENT_BASE - XIP_BASE, buf, write_size);
  //END Critical Section

  free(buf);
  return 0;
}

int read(char* key, char* out_val) {

}

