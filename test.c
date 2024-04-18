#include "test.h"
#include "connect.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "sensor.h"
#include <stdio.h>

#define I2C_TEST_BTN_PIN 19
#define I2C_TEST_LED_PIN 18
#define MQTT_TEST_BTN_PIN 14
#define MQTT_TEST_LED_PIN 15

int init_pins() {

  gpio_init(I2C_TEST_BTN_PIN);
  gpio_init(I2C_TEST_LED_PIN);
  gpio_init(MQTT_TEST_BTN_PIN);
  gpio_init(MQTT_TEST_LED_PIN);

  // Setup control pins
  gpio_set_dir(I2C_TEST_BTN_PIN, GPIO_IN);
  gpio_set_dir(I2C_TEST_LED_PIN, GPIO_OUT);
  gpio_set_dir(MQTT_TEST_BTN_PIN, GPIO_IN);
  gpio_set_dir(MQTT_TEST_LED_PIN, GPIO_OUT);
  cyw43_arch_gpio_put(I2C_TEST_LED_PIN, 1);
  cyw43_arch_gpio_put(MQTT_TEST_LED_PIN, 1);
  printf("GPIO Initialized\n");

  return 0;
}

void test() {
  init_pins();

  while (1) {
    if (!gpio_get(I2C_TEST_BTN_PIN)) {
      gpio_put(I2C_TEST_LED_PIN, true);

      printf("running i2c sensor test\n");
      if (sensor_test()) {
        printf("script failed during i2c sensor test\n");
      }

      sleep_ms(500);
    } else {
      gpio_put(I2C_TEST_LED_PIN, false);
    }

    if (!gpio_get(MQTT_TEST_BTN_PIN)) {
      gpio_put(MQTT_TEST_LED_PIN, true);

      printf("running MQTT connection test\n");
      if (mqtt_test(WIFI_SSID, WIFI_PASSWORD, MQTT_BROKER_IP)) {
        printf("script failed during MQTT connection test\n");
      }

      sleep_ms(500);
    } else {
      gpio_put(MQTT_TEST_LED_PIN, false);
    }
  }
}
