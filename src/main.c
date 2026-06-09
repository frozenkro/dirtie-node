#include "connect/connect.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "sensor/sensor.h"
#include "test.h"
#include <stdio.h>
#include "state/state.h"

#ifndef TEST_MODE
#define TEST_MODE 0
#endif

// Set in cmakelists
const char* HUB_LOC;

const int32_t SAMPLE_RATE = 5000; // 5 seconds

bool sample_rate_update(struct repeating_timer *t) {
  APP_CTX_T *ctx = t->user_data;
  ctx->set_publish = 1;

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
  stdio_init_all();

  if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA)) {
    printf("failed to initialize \n");
    return 1;
  }
  printf("CYW43 Initialized\n");

  APP_CTX_T *ctx = malloc(sizeof(APP_CTX_T));
  strcpy(ctx->hub_loc, HUB_LOC);

  // Regular publish interval
  struct repeating_timer timer;
  add_repeating_timer_ms(SAMPLE_RATE, sample_rate_update, ctx, &timer);

  while (1) {
    DT_ERR_E err = LOOP_STATE_CALLBACKS[ctx->loop_state](ctx);
    if (err) {
      printf("Error: %d\n", err);
    }

    update_state(ctx);

    // Check time/timer
    // Check sensor
    // // If diff between new reading and last published reading > 50,
    // // or if last published reading was 1 hour ago
    // // Add reading to event publish array

    // check battery
    // // If low, add reading to event publish array
    // // And turn on battery LED?

    // Handle usb 
    //
    // Handle MQTT subscriptions

    // Publish all events from array to MQTT broker

    sleep_ms(1000);
  }

  // If ever a graceful shutdown is required
  free(ctx);
/* #endif */
}
