#include "connect/connect.h"
#include "dt_globals.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "sensor/sensor.h"
#include "test.h"
#include <stdio.h>

#ifndef TEST_MODE
#define TEST_MODE 0
#endif

char* WIFI_SSID;
char* WIFI_PASSWORD;

struct loop_state {
  uint16_t last_published_capacitance;
  uint16_t last_published_temperature;
};

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
  struct loop_state *state = t->user_data;
  uint16_t cap;
  uint16_t temp;
  sensor_check(&cap, &temp);

  state->last_published_temperature = temp;
  state->last_published_capacitance = cap;

  // TODO publish results 
  return true;
}

int main() {
  stdio_init_all();

  if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA)) {
    printf("failed to initialize \n");
    return 1;
  }
  printf("CYW43 Initialized\n");

/* #if TEST_MODE == 1 */
/*   test(); */

/* #else */
  if (init_offline_modules()) {
    return 1;
  }

  // Await WIFI configuration over usb
  // TODO store to persistent storage and skip this 
  while (1) {
    if (WIFI_SSID[0] != '\0' && WIFI_PASSWORD[0] != '\0') {
      break;
    }
    sleep_ms(250);
  }
  
  struct loop_state loop_state;
  loop_state.last_published_capacitance = 0;
  loop_state.last_published_temperature = 0;
  
  struct repeating_timer timer;

  add_repeating_timer_ms(3600000, hourly_update, &loop_state, &timer);
  while (1) {
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
/* #endif */
}
