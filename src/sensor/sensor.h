#ifndef SENSOR_H
#define SENSOR_H

#include "dt_err.h"
#include <stdint.h>
#include "state/state.h"

int sensor_test();

DT_ERR_E sense_init_handler(APP_CTX_T *ctx);
DT_ERR_E sense_listen_handler(APP_CTX_T *ctx);

#endif
