#include "test.h"
#include "connect.h"
#include "dirtie_globals.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "sensor.h"
#include "usb_cfg.h"
#include <stdio.h>

#define MQTT_TEST_BTN_PIN 14
#define MQTT_TEST_LED_PIN 15
#define USB_TEST_BTN_PIN 16
#define USB_TEST_LED_PIN 17
#define I2C_TEST_BTN_PIN 19
#define I2C_TEST_LED_PIN 18
#define CANCEL_BTN_PIN 20

int init_pins() {

  gpio_init(MQTT_TEST_BTN_PIN);
  gpio_init(MQTT_TEST_LED_PIN);
  gpio_init(I2C_TEST_BTN_PIN);
  gpio_init(I2C_TEST_LED_PIN);
  gpio_init(USB_TEST_BTN_PIN);
  gpio_init(USB_TEST_LED_PIN);
  gpio_init(CANCEL_BTN_PIN);

  // Setup control pins
  gpio_set_dir(I2C_TEST_BTN_PIN, GPIO_IN);
  gpio_set_dir(I2C_TEST_LED_PIN, GPIO_OUT);
  gpio_set_dir(MQTT_TEST_BTN_PIN, GPIO_IN);
  gpio_set_dir(MQTT_TEST_LED_PIN, GPIO_OUT);
  gpio_set_dir(USB_TEST_BTN_PIN, GPIO_IN);
  gpio_set_dir(USB_TEST_LED_PIN, GPIO_OUT);
  gpio_set_dir(CANCEL_BTN_PIN, GPIO_IN);
  cyw43_arch_gpio_put(I2C_TEST_LED_PIN, 1);
  cyw43_arch_gpio_put(MQTT_TEST_LED_PIN, 1);
  cyw43_arch_gpio_put(USB_TEST_LED_PIN, 1);
  printf("GPIO Initialized\n");

  return 0;
}

int check_cancel_cb() { return !gpio_get(CANCEL_BTN_PIN); }

void test() {
  init_pins();

  while (1) {
    if (!gpio_get(I2C_TEST_BTN_PIN)) {
      gpio_put(I2C_TEST_LED_PIN, 1);

      printf("running i2c sensor test\n");
      if (sensor_test()) {
        printf("script failed during i2c sensor test\n");
      }

      sleep_ms(500);
    } else {
      gpio_put(I2C_TEST_LED_PIN, 0);
    }

    if (!gpio_get(USB_TEST_BTN_PIN)) {
      gpio_put(USB_TEST_LED_PIN, 1);

      printf("running usb cdc configuration test\n");
      if (!cfg_test(check_cancel_cb)) {
        printf("script failed during USB config test.\n");
      } else {
        printf("USB config test exited");
      }

      sleep_ms(500);
    } else {
      gpio_put(USB_TEST_LED_PIN, 0);
    }

    if (!gpio_get(MQTT_TEST_BTN_PIN)) {
      gpio_put(MQTT_TEST_LED_PIN, 1);

      printf("running MQTT connection test\n");
      if (!wifi_configured) {
        printf("wifi has not yet been set up");
      } else if (mqtt_test(WIFI_SSID, WIFI_PASSWORD, MQTT_BROKER_IP,
                           check_cancel_cb)) {
        printf("script failed during MQTT connection test\n");
      }

      sleep_ms(500);
    } else {
      gpio_put(MQTT_TEST_LED_PIN, 0);
    }
  }
}
