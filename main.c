#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "connect.h"
#include "pico/time.h"
#include "sensor.h"


#ifndef MQTT_BROKER_IP 
#define MQTT_BROKER_IP "0.0.0.0"
#endif

#define I2C_TEST_BTN_PIN 19
#define I2C_TEST_LED_PIN 18
#define MQTT_TEST_BTN_PIN 14
#define MQTT_TEST_LED_PIN 15

int main() {
  stdio_init_all();

  if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA)) {
    printf("failed to initialize \n");
    return 1;
  }
  printf("Initialized\n");

  // Setup control pins
  gpio_set_dir(I2C_TEST_BTN_PIN, GPIO_IN);
  gpio_set_dir(I2C_TEST_LED_PIN, GPIO_OUT);
  gpio_set_dir(MQTT_TEST_BTN_PIN, GPIO_IN);
  gpio_set_dir(MQTT_TEST_LED_PIN, GPIO_OUT);


  printf("I2c button get: %i\n", gpio_get(I2C_TEST_BTN_PIN));
  printf("I2c LED get: %i\n", gpio_get(I2C_TEST_LED_PIN));
  printf("mqtt button get: %i\n", gpio_get(MQTT_TEST_BTN_PIN));
  printf("mqtt LED get: %i\n", gpio_get(MQTT_TEST_LED_PIN));
  gpio_put(MQTT_TEST_LED_PIN, true);
  sleep_ms(250);
  printf("mqtt LED get: %i\n", gpio_get(MQTT_TEST_LED_PIN));

  while (1) {
    if (!gpio_get(I2C_TEST_BTN_PIN)) {
      gpio_put(I2C_TEST_LED_PIN, true);

      printf("running i2c sensor test\n");
      if (test_sensor()) {
        printf("script failed during i2c sensor test\n");
      }
      
      sleep_ms(500);
    } else {
      gpio_put(I2C_TEST_LED_PIN, false);
    }

    if (!gpio_get(MQTT_TEST_BTN_PIN)) {
      gpio_put(MQTT_TEST_LED_PIN, true);

      printf("running MQTT connection test\n");
      if (connect_mqtt(WIFI_SSID, WIFI_PASSWORD,
          MQTT_BROKER_IP)) {
        printf("script failed during MQTT connection test\n");
      }
      
      sleep_ms(500);
    } else {
      gpio_put(MQTT_TEST_LED_PIN, false);
    }
  }



  cyw43_arch_deinit();
  return 0;
}
