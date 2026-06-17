#ifndef LOGGER_H
#define LOGGER_H

#include "dt_err.h"
#include "state/state.h"

#define MAX_LOG_LEN 1024

typedef enum {
  LOG_INFO,
  LOG_ERR,
} LOG_LVL_E;

DT_ERR_E dlog(APP_CTX_T *ctx, LOG_LVL_E lvl, char* message, ...);
void free_logs(APP_CTX_T *ctx);
void log_transform_ts(char* msg, uint64_t now, uint64_t hwnow);

#endif
