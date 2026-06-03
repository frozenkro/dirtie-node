#ifndef CONNECT_H
#define CONNECT_H

#include "state/state.h"

int mqtt_init(APP_CTX_T *ctx);
int mqtt_check();

int mqtt_test(APP_CTX_T *ctx, int (*check_cancel_cb)());

#endif
