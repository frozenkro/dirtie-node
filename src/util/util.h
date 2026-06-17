#ifndef UTIL_H
#define UTIL_H

#include "dt_err.h"
#include "state/state.h"
#include <stdint.h>

typedef struct NTP_T_ NTP_T;

NTP_T* ntp_init(void);
void ntp_deinit(NTP_T *state);
void ntp_poll(NTP_T *state);

DT_ERR_E get_current_epoch(APP_CTX_T *ctx, uint64_t *result);

#endif
