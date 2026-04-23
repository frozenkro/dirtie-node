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

// char* WIFI_SSID;
// char* WIFI_PASSWORD;

int init_offline_modules() {
  if (sensor_init()) {
    printf("error occurred while initializing i2c sensor module");
    return 1;
  }
  return 0;
}

int init_network_modules() {
# ifndef MQTT_BROKER_IP
  printf("No MQTT_BROKER_IP defined, no data will be published");
  return 0;
# else 
  
  if (mqtt_init(WIFI_SSID, WIFI_PASSWORD, MQTT_BROKER_IP)) {
    printf("error occured while initializing mqtt connection");
    return 1;
  }
  return 0;
#endif
}

bool hourly_update(struct repeating_timer *t) {
  APP_CTX_T *ctx = t->user_data;
  ctx->set_publish = 1;

  return true;
}

DT_ERR_E throttle_listen_handler(APP_CTX_T* _) {
  sleep_ms(1000);
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

  // Regular publish interval
  struct repeating_timer timer;
  add_repeating_timer_ms(3600000, hourly_update, &ctx, &timer);

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
