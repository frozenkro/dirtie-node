#ifndef FLASHMEM_H
#define FLASHMEM_H

#include <stdint.h>
#include <stdbool.h>
#include "state/state.h"

// Defined in memmap_custom.ld
extern uint32_t ADDR_PERSISTENT[];
#define ADDR_PERSISTENT_BASE (ADDR_PERSISTENT)

// Can only write flash in pages of 256 bytes
#define PAGE_SIZE 256
#define MAX_VAL_SIZE 256
// flash sector alignment size
#define NVS_SIZE 4096

DT_ERR_E write(const char* key, const char* val);
DT_ERR_E read(const char* key, char* out_val);
DT_ERR_E initialize_flash(bool force_clear);

DT_ERR_E flash_init_handler(APP_CTX_T *ctx);
DT_ERR_E flash_write_handler(APP_CTX_T *ctx);

#endif
