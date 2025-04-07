#include "pico/stdlib.h"
#include "stdlib.h"
#include <lwip/netif.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "hardware/flash.h"
#include "flashmem.h"

// Defined in memmap_custom.ld
extern uint32_t ADDR_PERSISTENT[];
#define ADDR_PERSISTENT_BASE (ADDR_PERSISTENT)

// Can only write flash in pages of 256 bytes
#define PAGE_SIZE 256
// flash sector alignment size
#define NVS_SIZE 4096


// key1=val1,key2=val2,(etc..)\0
//
void initialize_flash() {
  uint8_t *buf = (uint8_t *)ADDR_PERSISTENT_BASE;
  for (size_t i = 0; i < NVS_SIZE; i++) {
    if (buf[i] != 0xFF) {
      return;
    }
  }

  flash_range_erase((uint32_t)ADDR_PERSISTENT_BASE - XIP_BASE, NVS_SIZE);
}

char* esc_chars(const char* val) {
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
      *head = '\\';
      head++;
    }
    *head = *src;
    head++;
  }
  *head = '\0';

  return res;
}

char* get_next_item(char *ptr) {
  while(*ptr != '\0') {
    if (*ptr == ',') {
      return ptr;
    }
    ptr++;
  }
  return NULL;
}

flashmem_err_t splice(char* buf, char* start, char* end, char* replace) {
  int start_idx = start - buf;
  int end_idx = strrchr(buf, '\0') - end;

  if (strlen(buf) + (start_idx - end_idx) + strlen(replace) > NVS_SIZE) {
    return FM_ERR_FLASHMEM_SIZE;
  }

  char* chunk_pre = malloc(start - buf);
  memcpy(chunk_pre, buf, start-buf);
  char* chunk_post = malloc(end_idx);
  memcpy(chunk_post, end, end_idx);

  memset(buf, 0, NVS_SIZE);
  memcpy(buf, chunk_pre, start_idx);
  memcpy(start, replace, strlen(replace));
  char* new_chunk_post_start = strrchr(start, '\0');
  memcpy(new_chunk_post_start, chunk_post, strlen(end));
  
  free(chunk_pre);
  free(chunk_post);

  return FM_ERR_OK;
}

flashmem_err_t upsert_key(const char* key, const char* val, char* buf) {
  if (strrchr(key, '\\') != NULL || strrchr(key, ',') != NULL || strrchr(key, '=') != NULL) {
    return FM_ERR_INVALID_KEY;
  }
  initialize_flash();

  char* clean_val = esc_chars(val);

  int keylen = strlen(key);
  char *key_search = malloc(keylen + 2);
  strcpy(key_search, key);
  key_search[keylen] = '=';
  key_search[keylen + 1] = '\0';

  char* key_val_item = malloc(strlen(key_search) + strlen(clean_val) + 1);
  strcpy(key_val_item, key_search);
  char* val_item = key_val_item + strlen(key_search);
  strcpy(val_item, clean_val);
  key_val_item[strlen(key_search) + strlen(clean_val) + 1] = '\0';

  char* key_loc = strstr(buf, key_search);

  // if key not present, just append
  if (key_loc == NULL) {
    if (strlen(key_search) + strlen(clean_val) + strlen(buf) + 1 > NVS_SIZE - 1) {
      return FM_ERR_FLASHMEM_SIZE;
    }

    char *end = strrchr(buf, '\0');
    memset(end, ',', 1);
    memcpy(++end, key_val_item, strlen(key_val_item) + 1);
  }
  else {
  // else splice new value in place
    char* next_item = get_next_item(key_loc);
    if (next_item == NULL) {
      char *end = strrchr(buf, '\0');
      memset(end, ',', 1);
      memcpy(++end, key_val_item, strlen(key_val_item) + 1);
    }
    else {
      splice(buf, key_loc, next_item, key_val_item);
    }
  }

  free(clean_val);
  free(key_search);
  return FM_ERR_OK;
}

flashmem_err_t write(const char* key, const char* val) {
  uint8_t *buf = malloc(NVS_SIZE);

  memcpy(buf, ADDR_PERSISTENT, NVS_SIZE);

  flashmem_err_t err = upsert_key(key, val, (char*)buf);
  if (err != FM_ERR_OK) {
    return err;
  }

  unsigned int mem_size = strlen((char*)buf);
  int write_size = (mem_size / 256) + (256 - (mem_size % 256));

  //START Critical Section
  flash_range_erase((uint32_t)ADDR_PERSISTENT_BASE - XIP_BASE, NVS_SIZE);
  flash_range_program((uint32_t)ADDR_PERSISTENT_BASE - XIP_BASE, buf, write_size);
  //END Critical Section

  free(buf);
  return FM_ERR_OK;
}

flashmem_err_t read(const char* key, char* out_val) {
  if (strrchr(key, '\\') != NULL || strrchr(key, ',') != NULL || strrchr(key, '=') != NULL) {
    return FM_ERR_INVALID_KEY;
  }

  int keylen = strlen(key);
  char *key_search = malloc(keylen + 2);
  strcpy(key_search, key);
  key_search[keylen + 1] = '=';
  key_search[keylen + 2] = '\0';

  char* key_loc = strstr((char*)ADDR_PERSISTENT, key_search);
  char* ass = strchr(key_loc, '=');
  if (ass == NULL) {
    return FM_ERR_CORRUPT;
  }

  char* val = ass + 1;
  char* val_end = strrchr(val, ',');
  if (val_end == NULL) {
    val_end = strrchr(val, '\0');
  }
  if (val_end == NULL) {
    return FM_ERR_CORRUPT;
  }

  memcpy(out_val, val, val_end - val);

  free(key_search);
  return FM_ERR_OK;
}

