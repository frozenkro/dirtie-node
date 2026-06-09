#include "state.h"
#include <stdio.h>
#include <stdlib.h>

extern DT_ERR_E flash_init_handler(APP_CTX_T*);
extern DT_ERR_E wifi_config_handler(APP_CTX_T*);
extern DT_ERR_E flash_write_handler(APP_CTX_T*);
extern DT_ERR_E mqtt_init_handler(APP_CTX_T*);
extern DT_ERR_E sense_init_handler(APP_CTX_T*);
extern DT_ERR_E mqtt_publish_handler(APP_CTX_T*);
extern DT_ERR_E mqtt_listen_handler(APP_CTX_T*);
extern DT_ERR_E batt_listen_handler(APP_CTX_T*);
extern DT_ERR_E sense_listen_handler(APP_CTX_T*);
extern DT_ERR_E throttle_listen_handler(APP_CTX_T*);

const loop_state_cb_t LOOP_STATE_CALLBACKS[LOOP_STATE_COUNT] = {
  [FLASH_INIT] = flash_init_handler,
  [WIFI_CONFIG] = wifi_config_handler,
  [FLASH_WRITE] = flash_write_handler,
  [MQTT_INIT] = mqtt_init_handler,
  [SENSE_INIT] = sense_init_handler,
  [MQTT_PUBLISH] = mqtt_publish_handler,
  [MQTT_LISTEN] = mqtt_listen_handler,
  [BATT_LISTEN] = batt_listen_handler,
  [SENSE_LISTEN] = sense_listen_handler,
  [THROTTLE] = throttle_listen_handler,
};

void update_state(APP_CTX_T *ctx) {
  switch (ctx->loop_state) {
  case FLASH_INIT:
    if (ctx->flash_initd) {
      if (ctx->wifi_configd) {
        ctx->loop_state = MQTT_INIT;
      } else {
        ctx->loop_state = WIFI_CONFIG;
      }
    }
  case WIFI_CONFIG:
    if (ctx->wifi_configd) {
      ctx->loop_state = FLASH_WRITE;
    }
    break;
  case FLASH_WRITE:
    if (ctx->flash_written) {
      ctx->loop_state = MQTT_INIT;
    }
    break;
  case MQTT_INIT:
    if (ctx->mqtt_initd) {
      ctx->loop_state = SENSE_INIT;
    }
    break;
  case SENSE_INIT:
    if (ctx->sense_initd) {
      ctx->loop_state = MQTT_PUBLISH;
    }
    break;
  case MQTT_PUBLISH:
    ctx->loop_state = MQTT_LISTEN;
    break;
  case MQTT_LISTEN:
    ctx->loop_state = BATT_LISTEN;
    break;
  case BATT_LISTEN:
    ctx->loop_state = SENSE_LISTEN;
    break;
  case SENSE_LISTEN:
    ctx->loop_state = THROTTLE;
    break;
  case THROTTLE:
    ctx->loop_state = MQTT_LISTEN;
    break;
  case LOOP_STATE_COUNT:
  default:
    printf("Invalid state: %d\n", ctx->loop_state);
  }

  if (ctx->set_publish 
      && ctx->wifi_configd 
      && ctx->sense_initd
      && ctx->mqtt_initd) {
    ctx->loop_state = MQTT_PUBLISH;
  }
}
