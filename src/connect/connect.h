#ifndef CONNECT_H
#define CONNECT_H

#include "state/state.h"

int mqtt_test(APP_CTX_T *ctx, int (*check_cancel_cb)());

DT_ERR_E mqtt_init_handler(APP_CTX_T *ctx);
DT_ERR_E mqtt_publish_handler(APP_CTX_T *ctx);
DT_ERR_E logdump_handler(APP_CTX_T *ctx);

#endif
