#include "pico/stdlib.h"
#include "stdlib.h"
#include <lwip/netif.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "hardware/flash.h"
#include "flashmem.h"


// key1=val1,key2=val2,(etc..)\0
DT_ERR_E initialize_flash(bool force_clear) {
  if (!force_clear) {
    uint8_t *buf = (uint8_t *)ADDR_PERSISTENT_BASE;
    for (size_t i = 0; i < NVS_SIZE; i++) {
      if (buf[i] != 0xFF) {
        return DT_ERR_OK;
      }
    }
  }


  uint8_t *zero_buf = malloc(NVS_SIZE);
  if (zero_buf == NULL) {
    return DT_ERR_MALLOC;
  }
  memset(zero_buf, 0, NVS_SIZE);

  //START Critical Section
  flash_range_erase((uint32_t)ADDR_PERSISTENT_BASE - XIP_BASE, NVS_SIZE);
  flash_range_program((uint32_t)ADDR_PERSISTENT_BASE - XIP_BASE, zero_buf, NVS_SIZE);
  //END Critical Section

  free(zero_buf);
  return DT_ERR_OK;
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

DT_ERR_E splice(char* buf, char* start, char* end, char* replace) {
  int start_sz = start - buf;
  int end_sz = strlen(end);

  if (strlen(buf) + (start_sz - end_sz) + strlen(replace) > NVS_SIZE) {
    return DT_ERR_FLASHMEMSIZE;
  }

  char* chunk_pre = malloc(start - buf);
  memcpy(chunk_pre, buf, start-buf);
  char* chunk_post = malloc(end_sz);
  memcpy(chunk_post, end, end_sz);

  memset(buf, 0, NVS_SIZE);
  memcpy(buf, chunk_pre, start_sz);
  memcpy(start, replace, strlen(replace));
  char* new_chunk_post_start = strchr(start, '\0');
  memcpy(new_chunk_post_start, chunk_post, end_sz);
  
  free(chunk_pre);
  free(chunk_post);

  return DT_ERR_OK;
}

DT_ERR_E upsert_key(const char* key, const char* val, char* buf) {
  if (strchr(key, '\\') != NULL || strchr(key, ',') != NULL || strchr(key, '=') != NULL) {
    return DT_ERR_INVALIDKEY;
  }
  initialize_flash(false);

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

  char* key_loc = strstr(buf, key_search);

  // if key not present, just append
  if (key_loc == NULL) {
    if (strlen(key_search) + strlen(clean_val) + strlen(buf) + 1 > NVS_SIZE - 1) {
      return DT_ERR_FLASHMEMSIZE;
    }

    char *end = strchr(buf, '\0');
    memset(end, ',', 1);
    memcpy(++end, key_val_item, strlen(key_val_item) + 1);
  }
  else {
    // else splice new value in place
    char* end = get_next_item(key_loc);
    if (end == NULL) {
      end = strchr(buf, '\0');
    }
    splice(buf, key_loc, end, key_val_item);
  }

  free(clean_val);
  free(key_search);
  return DT_ERR_OK;
}

DT_ERR_E write(const char* key, const char* val) {
  if (strlen(val) > MAX_VAL_SIZE) {
    return DT_ERR_VALSIZE;
  }

  uint8_t *buf = malloc(NVS_SIZE);
  memcpy(buf, ADDR_PERSISTENT, NVS_SIZE);

  DT_ERR_E err = upsert_key(key, val, (char*)buf);
  if (err != DT_ERR_OK) {
    return err;
  }

  unsigned int mem_size = strlen((char*)buf);
  int write_size = (mem_size / 256) + (256 - (mem_size % 256));

  //START Critical Section
  flash_range_erase((uint32_t)ADDR_PERSISTENT_BASE - XIP_BASE, NVS_SIZE);
  flash_range_program((uint32_t)ADDR_PERSISTENT_BASE - XIP_BASE, buf, write_size);
  //END Critical Section

  free(buf);
  return DT_ERR_OK;
}

DT_ERR_E get_val_end(char* val, char** val_end) {
  *val_end = NULL;
  
  for (int i = 0; i < strlen(val); i++) {
    if (val[i] == '\\') {
      i++;
    }
    if (val[i] == ',') {
      *val_end = val + i;
      break;
    }
  }

  if (*val_end == NULL) {
    *val_end = strchr(val, '\0');
  }
  if (*val_end == NULL) {
    return DT_ERR_CORRUPT;
  }
  return DT_ERR_OK;
}

DT_ERR_E read(const char* key, char* out_val) {
  if (strchr(key, '\\') != NULL || strchr(key, ',') != NULL || strchr(key, '=') != NULL) {
    return DT_ERR_INVALIDKEY;
  }

  int keylen = strlen(key);
  char *key_search = malloc(keylen + 2);
  strcpy(key_search, key);
  key_search[keylen] = '=';
  key_search[keylen + 1] = '\0';

  char* key_loc = strstr((char*)ADDR_PERSISTENT, key_search);
  if (key_loc == NULL) {
    free(key_search);
    return DT_ERR_KEYNOTFOUND;
  }

  char* ass = strchr(key_loc, '=');
  if (ass == NULL) {
    return DT_ERR_CORRUPT;
  }

  char* val = ass + 1;
  char* val_end;
  DT_ERR_E err = get_val_end(val, &val_end);
  if (err != DT_ERR_OK) {
    return err;
  }

  memcpy(out_val, val, val_end - val);
  out_val[val_end - val] = 0;

  char* esc_char = out_val;
  char* end = out_val + strlen(out_val);
  while (esc_char < end) {
    if (*esc_char == '\\') {
      memmove(esc_char, esc_char + 1, strlen(esc_char + 1) + 1);
      end--;
    }
    esc_char++;
  }

  free(key_search);
  return DT_ERR_OK;
}

DT_ERR_E flash_init_handler(APP_CTX_T *ctx) {
  DT_ERR_E err = initialize_flash(false);
  if (err != DT_ERR_OK) { return err; }

  err = read("ssid", ctx->wifi_ssid);
  if (err == DT_ERR_KEYNOTFOUND) { 
    // WIFI not yet configured
    ctx->flash_initd = true;
    return DT_ERR_OK;
  } else if (err != DT_ERR_OK) {
    return err;
  }

  err = read("password", ctx->wifi_pass);
  if (err != DT_ERR_OK) {
    return err;
  }

  // SSID and password restored from flash
  ctx->wifi_configd = true;
  ctx->flash_initd = true;

  return DT_ERR_OK;
}

DT_ERR_E flash_write_handler(APP_CTX_T *ctx) {
  DT_ERR_E err = write("ssid", ctx->wifi_ssid);
  if (err != DT_ERR_OK) {
    return err;
  }

  err = write("password", ctx->wifi_pass);
  if (err != DT_ERR_OK) {
    return err;
  }

  ctx->wifi_configd = true;

  return DT_ERR_OK;
}

