#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "connect.h"
#include "sensor.h"


#ifndef MQTT_BROKER_IP 
#define MQTT_BROKER_IP "0.0.0.0"
#endif

#define DEBUG_printf printf

int main() {
  stdio_init_all();

  if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA)) {
    DEBUG_printf("failed to initialize \n");
    return 1;
  }
  DEBUG_printf("Initialized\n");

  DEBUG_printf("running i2c sensor test");
  if (test_sensor()) {
    DEBUG_printf("script failed during i2c sensor test");
    return 1;
  }

  DEBUG_printf("running MQTT connection test");
  if (connect_mqtt(WIFI_SSID, WIFI_PASSWORD,
      MQTT_BROKER_IP)) {
    DEBUG_printf("script failed during MQTT connection test");
    return 1;
  }

  cyw43_arch_deinit();
  return 0;
}
