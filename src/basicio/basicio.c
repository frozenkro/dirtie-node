
#include "basicio.h"
#include "hardware/gpio.h"
#include <pico/time.h>
#include <pico/types.h>
#include "pico/cyw43_arch.h"

#define RESET_BUTTON_MS 3000
#define BLINK_INTERVAL_AP_MODE_MS 1000
#define RESET_BUTTON_PIN 16

DT_ERR_E basicio_init_handler(APP_CTX_T *ctx) {
  gpio_init(RESET_BUTTON_PIN);
  gpio_set_dir(RESET_BUTTON_PIN, GPIO_IN);
}

DT_ERR_E basicio_listen_handler(APP_CTX_T *ctx) {

  // Handle blinking
  if (ctx->loop_state == WIFI_CONFIG && 
      absolute_time_diff_us(ctx->blink_ts, get_absolute_time()) > (BLINK_INTERVAL_AP_MODE_MS * 1000)) {
    if (ctx->blink_on) {
      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
      ctx->blink_on = 0;
    } 
    else {
      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
      ctx->blink_on = 1;
    }

    ctx->blink_ts = get_absolute_time();
  }

  // Handle reset btn
  if (!gpio_get(RESET_BUTTON_PIN)) {
    if (!ctx->reset_pressed) {
      ctx->reset_pressed = 1;
      ctx->reset_pressed_ts = get_absolute_time();
    }

    if (absolute_time_diff_us(ctx->reset_pressed_ts, get_absolute_time()) > (RESET_BUTTON_MS * 1000)) {
      ctx->reset = 1;
    }
  } 
  else {
    ctx->reset_pressed = 0;
  }

}
