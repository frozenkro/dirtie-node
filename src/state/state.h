#ifndef STATE_H 
#define STATE_H

#include "../dt_err.h"
#include <stdint.h>

typedef enum {
  WIFI_CONFIG,
  MQTT_INIT,
  SENSE_INIT,
  MQTT_PUBLISH,
  MQTT_LISTEN,
  BATT_LISTEN,
  SENSE_LISTEN,
  THROTTLE,
  LOOP_STATE_COUNT,
} LOOP_STATE_E;

typedef struct {
  LOOP_STATE_E loop_state;

  int set_publish;
  int wifi_configd;
  int mqtt_initd;
  int sense_initd;

  char *wifi_ssid;
  char *wifi_pass;
  
  uint16_t capacitance;
  uint16_t temperature;
  int16_t cap_diff;
  int16_t temp_diff;

} APP_CTX_T;

typedef DT_ERR_E (*loop_state_cb_t)(APP_CTX_T*);
extern const loop_state_cb_t LOOP_STATE_CALLBACKS[LOOP_STATE_COUNT];
extern void update_state(APP_CTX_T*);

#endif
