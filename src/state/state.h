#ifndef STATE_H 
#define STATE_H

#include "../dt_err.h"
#include <pico/types.h>
#include <stdint.h>

typedef struct MQTT_CLIENT_T_ MQTT_CLIENT_T;
typedef struct NTP_T_ NTP_T;

typedef enum {
  FLASH_INIT, 
  WIFI_CONFIG, // receive wifi credentials and provisioning token from mobile device 
  FLASH_WRITE, // write wifi credentials to flash mem
  MQTT_INIT, // complete provisioning with token from previous state
  SENSE_INIT, // initialize sensor hardware connection
  NTP_SYNC, // synchronize wall-clock time via NTP
  MQTT_PUBLISH,
  MQTT_LISTEN,
  BATT_LISTEN,
  SENSE_LISTEN,
  LOGDUMP,
  THROTTLE,
  LOOP_STATE_COUNT,
} LOOP_STATE_E;

typedef struct {
  LOOP_STATE_E loop_state;

  int set_publish;
  int flash_initd;
  int wifi_configd;
  int flash_written;
  int mqtt_initd;
  int sense_initd;
  int ntp_syncd;

  char wifi_ssid[64];
  char wifi_pass[64];
  char prv_token[64]; // provisioning contract for dirtie-srv

  char hub_loc[64]; // location of dirtie-srv + mqtt broker

  MQTT_CLIENT_T *mqtt_client;
  NTP_T *ntp_state;

  uint16_t capacitance;
  uint16_t temperature;
  uint16_t cprev;
  uint16_t tprev;

  int reset;
  absolute_time_t reset_pressed_ts;
  int reset_pressed;
  absolute_time_t blink_ts;
  int blink_on;

  int dump_logs;
  char* logdump[16];
} APP_CTX_T;

typedef DT_ERR_E (*loop_state_cb_t)(APP_CTX_T*);
extern const loop_state_cb_t LOOP_STATE_CALLBACKS[LOOP_STATE_COUNT];
extern void update_state(APP_CTX_T*);

#endif
