#include "pico/stdlib.h"
#include "stdlib.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "hardware/flash.h"

// Defined in memmap_custom.ld
extern uint32_t ADDR_PERSISTENT[];
#define ADDR_PERSISTENT_BASE (ADDR_PERSISTENT)

// Can only write flash in pages of 256 bytes
#define PAGE_SIZE 256
// flash sector alignment size
#define NVS_SIZE 4096

// key1=val1,key2=val2,(etc..)\0

int upsert_key(char* key, char* val, char* buf) {
  //TODO
  // if key not present, just append
  // else splice new value
  // return 1 if value exceeds 4096
  //
  return 0;
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

