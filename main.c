#include "connect.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "sensor.h"
#include "test.h"
#include <stdio.h>

#ifndef TEST_MODE
#define TEST_MODE 0
#endif

int main() {
  stdio_init_all();

  if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA)) {
    printf("failed to initialize \n");
    return 1;
  }
  printf("CYW43 Initialized\n");

#if TEST_MODE == 1
  test();

#else
  uint16_t last_published_capacity = 0;
  while (1) {
    // Check time/timer
    // Check sensor
    // // If diff between new reading and last published reading > 50,
    // // or if last published reading was 1 hour ago
    // // Add reading to event publish array

    // check battery
    // // If low, add reading to event publish array
    // // And turn on battery LED?

    // Handle bluetooth
    //
    // Handle MQTT subscriptions

    // Publish all events from array to MQTT broker
  }
#endif
}
