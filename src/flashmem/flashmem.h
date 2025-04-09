#ifndef FLASHMEM_H
#define FLASHMEM_H

#include <stdint.h>

// Defined in memmap_custom.ld
extern uint32_t ADDR_PERSISTENT[];
#define ADDR_PERSISTENT_BASE (ADDR_PERSISTENT)

// Can only write flash in pages of 256 bytes
#define PAGE_SIZE 256
#define MAX_VAL_SIZE 256
// flash sector alignment size
#define NVS_SIZE 4096

enum _flashmem_err 
{
  FM_ERR_OK = 0,
  FM_ERR_FLASHMEM_SIZE = 1,
  FM_ERR_INVALID_KEY = 2,
  FM_ERR_CORRUPT = 3,
  FM_ERR_MALLOC = 4,
  FM_ERR_VAL_SIZE = 5,
};
typedef enum _flashmem_err flashmem_err_t;

flashmem_err_t write(const char* key, const char* val);
flashmem_err_t read(const char* key, char* out_val);

#endif
