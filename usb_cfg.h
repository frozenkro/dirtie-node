#ifndef USB_CFG_H
#define USB_CFG_H

#include <stdbool.h>
bool WIFI_CONFIGURED = 0;

int cfg_test(int (*check_cancel_cb)());

void cfg_init();
int cfg_check();

#endif
