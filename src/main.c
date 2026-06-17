// #error TODO persist contract to flash, use in all communication with dirtie-srv
// #error TODO fix failure to return http provision request to client app properly
// #error TODO finish implementing logger.c (logdump lifecycle) and implement on dirtie-srv (send right to grafana?)

#include "connect/connect.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "sensor/sensor.h"
#include "test.h"
#include <stdio.h>
#include "state/state.h"
#include "basicio/basicio.h"
#include "logger/logger.h"
#include "util/util.h"

#ifndef TEST_MODE
#define TEST_MODE 0
#endif

// Set in cmakelists
// const char* HUB_LOC;

const int32_t SAMPLE_RATE = 5000; // 5 seconds

bool sample_rate_update(struct repeating_timer *t) {
  APP_CTX_T *ctx = t->user_data;
  if (ctx->mqtt_initd) {
    ctx->set_publish = 1;
  }

  return true;
}

DT_ERR_E throttle_listen_handler(APP_CTX_T* _) {
  sleep_ms(100);
  return DT_ERR_OK;
}

DT_ERR_E batt_listen_handler(APP_CTX_T *_) {
  // Have not yet implemented battery sensing
  return DT_ERR_OK;
}


int main() {
  // For debugging so sleep doesn't hang 
  timer_hw->dbgpause = 0;

  APP_CTX_T *ctx = calloc(sizeof(APP_CTX_T), 1);
  strcpy(ctx->hub_loc, HUB_LOC);
  dlog(ctx, LOG_INFO, "HUB_LOC set to %s\n", HUB_LOC);

  stdio_init_all();

  if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA)) {
    dlog(ctx, LOG_ERR, "failed to initialize \n");
    return 1;
  }
  dlog(ctx, LOG_INFO, "CYW43 Initialized\n");
  
  // Regular publish interval
  struct repeating_timer timer;
  add_repeating_timer_ms(SAMPLE_RATE, sample_rate_update, ctx, &timer);

  basicio_init_handler(ctx);

  while (1) {
    DT_ERR_E err = LOOP_STATE_CALLBACKS[ctx->loop_state](ctx);
    if (err) {
      dlog(ctx, LOG_ERR, "%d\n", err);
    }

    update_state(ctx);

    // check battery
    // // If low, add reading to event publish array
    // // And turn on battery LED?

    // Handle button + LED
    basicio_init_handler(ctx);
  }

  // If ever a graceful shutdown is required
  free(ctx);
/* #endif */
}
