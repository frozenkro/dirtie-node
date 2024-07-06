#include "usb_cfg.h"
#include "bsp/board.h"
#include "dirtie_config.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include <stdlib.h>
#include <string.h>

#define MAX_SSID_LEN 32
#define MAX_WPA2_PW_LEN 64

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
  return 0;
}

size_t get_console_inputs(uint8_t *buf, size_t bufsize) {
  size_t count = 0;
  while (count < bufsize) {
    int ch = board_getchar();
    if (ch <= 0)
      break;

    buf[count] = (uint8_t)ch;
    count++;
  }

  return count;
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

  tuh_init(BOARD_TUH_RHPORT);
}

int cfg_check() {
  tuh_task();

  uint8_t buf[64 + 1]; // +1 for extra null character
  uint32_t const bufsize = sizeof(buf) - 1;

  uint32_t count = get_console_inputs(buf, bufsize);
  buf[count] = 0;

  // loop over all mounted interfaces
  for (uint8_t idx = 0; idx < CFG_TUH_CDC; idx++) {
    if (tuh_cdc_mounted(idx)) {
      // console --> cdc interfaces
      if (count) {
        tuh_cdc_write(idx, buf, count);
        tuh_cdc_write_flush(idx);
      }
    }
  }

  return 0;
}

// Invoked when received new data
void tuh_cdc_rx_cb(uint8_t idx) {
  uint8_t buf[64 + 1]; // +1 for extra null character
  uint32_t const bufsize = sizeof(buf) - 1;

  // forward cdc interfaces -> console
  uint32_t count = tuh_cdc_read(idx, buf, bufsize);
  buf[count] = 0;

  printf("%s", buf);
  handle_command(buf);
}

void tuh_cdc_mount_cb(uint8_t idx) {
  tuh_cdc_itf_info_t itf_info = {0};
  tuh_cdc_itf_get_info(idx, &itf_info);

  printf("CDC Interface is mounted: address = %u, itf_num = %u\r\n",
         itf_info.daddr, itf_info.bInterfaceNumber);

#ifdef CFG_TUH_CDC_LINE_CODING_ON_ENUM
  // CFG_TUH_CDC_LINE_CODING_ON_ENUM must be defined for line coding is set by
  // tinyusb in enumeration otherwise you need to call tuh_cdc_set_line_coding()
  // first
  cdc_line_coding_t line_coding = {0};
  if (tuh_cdc_get_local_line_coding(idx, &line_coding)) {
    printf("  Baudrate: %u, Stop Bits : %u\r\n", line_coding.bit_rate,
           line_coding.stop_bits);
    printf("  Parity  : %u, Data Width: %u\r\n", line_coding.parity,
           line_coding.data_bits);
  }
#endif
}

void tuh_cdc_umount_cb(uint8_t idx) {
  tuh_cdc_itf_info_t itf_info = {0};
  tuh_cdc_itf_get_info(idx, &itf_info);

  printf("CDC Interface is unmounted: address = %u, itf_num = %u\r\n",
         itf_info.daddr, itf_info.bInterfaceNumber);
}
