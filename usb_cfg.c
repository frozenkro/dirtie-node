#include "usb_cfg.h"
#include "bsp/board.h"
#include "dirtie_config.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include <stdlib.h>
#include <string.h>

#define MAX_SSID_LEN 32
#define MAX_WPA2_PW_LEN 64

void handle_command(const char *cmd) {
  if (strncmp(cmd, "SET_SSID", 8) == 0) {
    const char *ssid = cmd + 9;
    strncpy(WIFI_SSID, ssid, MAX_SSID_LEN - 1);
    WIFI_SSID[MAX_SSID_LEN - 1] = '\0';
  } else if (strncmp(cmd, "SET_PASS", 8) == 0) {
    const char *pass = cmd + 9;
    strncpy(WIFI_PASSWORD, pass, MAX_WPA2_PW_LEN - 1);
    WIFI_PASSWORD[MAX_WPA2_PW_LEN - 1] = '\0';
  } else {
    printf("Unrecognized command %s\n", cmd);
  }
}

int cfg_init() {
  WIFI_SSID = calloc(MAX_SSID_LEN + 1, 1);
  WIFI_PASSWORD = calloc(MAX_WPA2_PW_LEN + 1, 1);

  bool err = tusb_init();
  if (err) {
    return 1;
  }
  return 0;
}

int cfg_check() {
  tud_task();

  uint32_t buf_size = 256;
  char buf[256 + 1];

  // check for cdc messages sent over usb and call handle_command
  if (tud_cdc_connected() && tud_cdc_available()) {
    tud_cdc_read(buf, buf_size);
    handle_command(buf);
  }
}
