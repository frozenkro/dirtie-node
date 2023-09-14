#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int main() {
  stdio_init_all();

  if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA)) {
    printf("failed to initialize \n");
    return 1;
  }

  printf("Initialized\n");

  cyw43_arch_enable_sta_mode();

  if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)){
    printf("failed to connect \n");
    return 1;
  }
  
  printf("connected!\n");
}
