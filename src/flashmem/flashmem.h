#ifndef FLASHMEM_H
#define FLASHMEM_H

enum _flashmem_err 
{
  FM_ERR_OK = 0,
  FM_ERR_FLASHMEM_SIZE = 1,
  FM_ERR_INVALID_KEY = 2,
  FM_ERR_CORRUPT = 3,
  FM_ERR_MALLOC = 4,
};
typedef enum _flashmem_err flashmem_err_t;

flashmem_err_t write(const char* key, const char* val);
flashmem_err_t read(const char* key, char* out_val);

#endif
