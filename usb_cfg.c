#include "usb_cfg.h"
#include "bsp/board.h"
#include "dirtie_globals.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include <stdlib.h>
#include <string.h>

#define MAX_SSID_LEN 32
#define MAX_WPA2_PW_LEN 64

int wifi_configured = 0;
char *WIFI_SSID;
char *WIFI_PASSWORD;

enum { BLINK_NOT_MOUNTED = 250, BLINK_MOUNTED = 1000, BLINK_SUSPENDED = 2500 };

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

int handle_command(const char *cmd) {
  if (strncmp(cmd, "SET_SSID", 8) == 0) {
    const char *ssid = cmd + 9;
    strncpy(WIFI_SSID, ssid, MAX_SSID_LEN - 1);
    WIFI_SSID[MAX_SSID_LEN - 1] = '\0';
    printf("Setting SSID to '%s'", ssid);
  } else if (strncmp(cmd, "SET_PASS", 8) == 0) {
    const char *pass = cmd + 9;
    strncpy(WIFI_PASSWORD, pass, MAX_WPA2_PW_LEN - 1);
    WIFI_PASSWORD[MAX_WPA2_PW_LEN - 1] = '\0';
    printf("Setting password to '%s'", pass);
  } else {
    printf("Unrecognized command %s\n", cmd);
    return 1;
  }

  if (WIFI_SSID[0] != '\0' && WIFI_PASSWORD[0] != '\0') {
    wifi_configured = 1;
  }

  return 0;
}

void led_blinking_task(void) {
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if (board_millis() - start_ms < blink_interval_ms)
    return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}

int cfg_test(int (*check_cancel_cb)()) {
  cfg_init();

  while (1) {
    if (cfg_check() != 0) {
      return 1;
    } else if (check_cancel_cb()) {
      return 0;
    }
  }

  return 0;
}

void cfg_init() {
  WIFI_SSID = calloc(MAX_SSID_LEN + 1, 1);
  WIFI_PASSWORD = calloc(MAX_WPA2_PW_LEN + 1, 1);

  tud_init(BOARD_TUD_RHPORT);
}

int cfg_check() {
  tud_task();
  led_blinking_task();

  if (tud_cdc_available()) {
    char buf[64];
    uint32_t count = tud_cdc_read(buf, sizeof(buf));
    (void)count;

    handle_command(buf);

    if (TEST_MODE) {
      // echo back
      tud_cdc_write(buf, count);
      tud_cdc_write_flush();
    }
  }

  return 0;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) { blink_interval_ms = BLINK_MOUNTED; }

// Invoked when device is unmounted
void tud_umount_cb(void) { blink_interval_ms = BLINK_NOT_MOUNTED; }

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
  (void)remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
  (void)itf;
  (void)rts;

  // TODO set some indicator
  if (dtr) {
    // Terminal connected
  } else {
    // Terminal disconnected
  }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf) { (void)itf; }
