#include "connect.h"
#include "dt_err.h"
#include "lwip/apps/mqtt.h"
#include "lwip/ip4_addr.h"
#include "lwip/ip_addr.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include "pico/types.h"
#include <cyw43.h>
#include <cyw43_ll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "state/state.h"

char TOPIC_BREADCRUMB[] = "dirtie-breadcrumb";
char TOPIC_PROVISION[] = "dirtie-provision";

int (*DEBUG_CHECK_CANCEL_CB)();

struct MQTT_CLIENT_T_ {
  ip_addr_t remote_addr;
  mqtt_client_t *mqtt_client;
  u32_t received;
  u32_t counter;
  u32_t reconnect;
};

err_t mqtt_test_connect(MQTT_CLIENT_T *state);

// Perform initialisation
static MQTT_CLIENT_T *mqtt_client_init(void) {
  MQTT_CLIENT_T *state = calloc(1, sizeof(MQTT_CLIENT_T));
  if (!state) {
    printf("failed to allocate state\n");
    return NULL;
  }
  state->received = 0;
  return state;
}

// note that buffer len is set to 1025 instead of 1024
// this is to account for the null terminator of
// the string, in the event that the buffer turns
// out to be exactly 1024 bytes
u8_t buffer[1025];
u8_t data_len = 0;
u32_t data_in = 0;

static void mqtt_pub_start_cb(void *arg, const char *topic, u32_t tot_len) {
  printf("mqtt_pub_start_cb called for topic: \"%s\"\n", topic);

  if (tot_len > 1024) {
    printf("Message length exceeds buffer size, discarding");
  } else {
    data_in = tot_len;
    data_len = 0;
  }
}

static void mqtt_pub_data_cb(void *arg, const u8_t *data, u16_t len,
                             u8_t flags) {
  if (data_in > 0) {
    data_in -= len;
    memcpy(&buffer[data_len], data, len);
    data_len += len;

    if (data_in == 0) {
      buffer[data_len] = 0;
      printf("Message received: \"%s\"\n", &buffer);
    }
  }
}

void mqtt_pub_request_cb(void *arg, err_t err) {
  MQTT_CLIENT_T *state = (MQTT_CLIENT_T *)arg;
  printf("mqtt_pub_request_cb: err %d\n", err);
  state->received++;
}

void mqtt_sub_request_cb(void *arg, err_t err) {
  printf("mqtt_sub_request_cb: err %d\n", err);
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg,
                               mqtt_connection_status_t status) {
  if (status != 0) {
    printf("Error during connection: err %d.\n", status);
  } else {
    printf("MQTT connected.\n");
  }
}

err_t mqtt_test_publish(MQTT_CLIENT_T *state) {
  char testbuf[128];

  sprintf(testbuf, "{\"message\":\"test dirtie node %d / %d \"}",
          state->received, state->counter);

  err_t err;
  u8_t qos =
      0; /* 0 1 or 2, see MQTT specification.  AWS IoT does not support QoS 2 */
  u8_t retain = 0;

  cyw43_arch_lwip_begin();
  err = mqtt_publish(state->mqtt_client, "pico_w/test", testbuf, strlen(testbuf),
                     qos, retain, mqtt_pub_request_cb, state);

  cyw43_arch_lwip_end();

  if (err != ERR_OK) {
    printf("Publish err: %d\n", err);
  }

  return err;
}


DT_ERR_E mqtt_connect(MQTT_CLIENT_T *state) {
  struct mqtt_connect_client_info_t ci;
  err_t lwip_err;

  memset(&ci, 0, sizeof(ci));

  uint8_t mac[6];
  cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA, mac);
  ci.client_id = mac;
  ci.client_user = NULL;
  ci.client_pass = NULL;
  ci.keep_alive = 0;
  ci.will_topic = NULL;
  ci.will_msg = NULL;
  ci.will_retain = 0;
  ci.will_qos = 0;

  // Skipping TLS for now

  const struct mqtt_connect_client_info_t *client_info = &ci;

  lwip_err = mqtt_client_connect(state->mqtt_client, &(state->remote_addr),
                            MQTT_PORT, mqtt_connection_cb, state, client_info);

  if (lwip_err != ERR_OK) {
    printf("mqtt_connect error %d\n", lwip_err);
    return DT_ERR_LWIPERR;
  }

  return DT_ERR_OK;
}

err_t mqtt_test_connect(MQTT_CLIENT_T *state) {
  struct mqtt_connect_client_info_t ci;
  err_t err;

  memset(&ci, 0, sizeof(ci));

  ci.client_id = "KTestPicoW";
  ci.client_user = NULL;
  ci.client_pass = NULL;
  ci.keep_alive = 0;
  ci.will_topic = NULL;
  ci.will_msg = NULL;
  ci.will_retain = 0;
  ci.will_qos = 0;

  const struct mqtt_connect_client_info_t *client_info = &ci;

  err = mqtt_client_connect(state->mqtt_client, &(state->remote_addr),
                            MQTT_PORT, mqtt_connection_cb, state, client_info);

  if (err != ERR_OK) {
    printf("mqtt_connect error %d\n", err);
  }

  return err;
}

int mqtt_run_test(MQTT_CLIENT_T *state) {
  // Create lwip mqtt client
  state->mqtt_client = mqtt_client_new();

  state->counter = 0;

  if (state->mqtt_client == NULL) {
    printf("Failed to create MQTT client\n");
    return 1;
  }

  if (mqtt_test_connect(state) == ERR_OK) {
    absolute_time_t timeout = nil_time;
    int subscribed = 0;
    mqtt_set_inpub_callback(state->mqtt_client, mqtt_pub_start_cb,
                            mqtt_pub_data_cb, 0);

    while (1) {
      cyw43_arch_poll();
      absolute_time_t now = get_absolute_time();
      if (is_nil_time(timeout) || absolute_time_diff_us(now, timeout) <= 0) {
        if (mqtt_client_is_connected(state->mqtt_client)) {
          cyw43_arch_lwip_begin();

          if (subscribed == 0) {
            mqtt_sub_unsub(state->mqtt_client, "pico_w/recv", 0,
                           mqtt_sub_request_cb, 0, 1);
            subscribed = 1;
          }

          if (mqtt_test_publish(state) == ERR_OK) {
            if (state->counter != 0) {
              printf("published %d\n", state->counter);
            }
            timeout = make_timeout_time_ms(5000);
            state->counter++;
          }
          cyw43_arch_lwip_end();
        }
      }
      sleep_ms(1000);
      if (DEBUG_CHECK_CANCEL_CB && DEBUG_CHECK_CANCEL_CB()) {
        return 0;
      }
    }
  }

  return 0;
}

int mqtt_test(APP_CTX_T *ctx, int (*check_cancel_cb)()) {
  // put in station mode because we are making connections from device
  if (!ctx->wifi_configd) {
    printf("wifi credentials not yet configured\n");
    return 1;
  }
  DEBUG_CHECK_CANCEL_CB = check_cancel_cb;

  cyw43_arch_enable_sta_mode();

  printf("connecting to %s\n", ctx->wifi_ssid);
  if (cyw43_arch_wifi_connect_timeout_ms(ctx->wifi_ssid, ctx->wifi_pass,
                                         CYW43_AUTH_WPA2_AES_PSK, 30000)) {
    printf("failed to connect.\n");
    return 1;
  } else {
    printf("Connected.\n");
  }

  MQTT_CLIENT_T *state = mqtt_client_init();

  // assign ip to state based on string
  int succ = ip4addr_aton(ctx->hub_loc, &state->remote_addr);
  if (!succ) {
    printf("Invalid IP String %s, exiting\n", ctx->hub_loc);
    return 1;
  }

  if (mqtt_run_test(state)) {
    printf("MQTT Test Failed, exiting\n");
    return 1;
  }

  return 0;
}

DT_ERR_E publish(MQTT_CLIENT_T *state, char *topic, char *message) {
  char msgbuf[128];
  strcpy(msgbuf, message);

  err_t lwip_err;
  u8_t qos =
      0; /* 0 1 or 2, see MQTT spec */
  u8_t retain = 0;

  cyw43_arch_lwip_begin();
  lwip_err = mqtt_publish(state->mqtt_client, topic, msgbuf, strlen(msgbuf),
                     qos, retain, mqtt_pub_request_cb, state);

  cyw43_arch_lwip_end();

  if (lwip_err != ERR_OK) {
    printf("Publish err: %d\n", lwip_err);
    return DT_ERR_LWIPERR;
  }

  return DT_ERR_OK;
}

DT_ERR_E provision(APP_CTX_T *ctx) {
  // create json payload
  // {"macAddr":"...","contract":"..."}

  uint8_t mac[6];
  cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA, mac);

  char *payload = malloc(45 + strlen(ctx->prv_token) + 1);
  if (payload == NULL) {
    return DT_ERR_MALLOC;
  }

  snprintf(payload, 45 + strlen(ctx->prv_token), "{\"macAddr\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\"contract\":\"%s\"}",
      mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], ctx->prv_token);

  DT_ERR_E err = publish(ctx->mqtt_client, TOPIC_PROVISION, payload);
  return err;
}

DT_ERR_E breadcrumb(APP_CTX_T *ctx) {

  uint8_t mac[6];
  cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA, mac);

  const char *template = "{\"macAddr\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\"capacitance\":\"%d\",\"temperature\":\"%d\"}";
  int len = snprintf(NULL, 0, template, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], ctx->capacitance, ctx->temperature);
  char *payload = malloc(len);
  if (payload == NULL) {
    return DT_ERR_MALLOC;
  }
  snprintf(payload, len, template, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], ctx->capacitance, ctx->temperature);

  DT_ERR_E err = publish(ctx->mqtt_client, TOPIC_BREADCRUMB, payload);
  return err;
}

DT_ERR_E mqtt_init_handler(APP_CTX_T *ctx) {
  if (!ctx->wifi_configd) {
    printf("wifi credentials not yet configured\n");
    return 1;
  }

  // put in station mode because we are making connections from device
  cyw43_arch_enable_sta_mode();

  printf("connecting to %s\n", ctx->wifi_ssid);
  if (cyw43_arch_wifi_connect_timeout_ms(ctx->wifi_ssid, ctx->wifi_pass,
                                         CYW43_AUTH_WPA2_AES_PSK, 30000)) {
    printf("failed to connect.\n");
    return 1;
  } else {
    printf("Connected.\n");
  }

  MQTT_CLIENT_T *state = mqtt_client_init();
  ctx->mqtt_client = state;

  // assign ip to state based on string
  int succ = ip4addr_aton(ctx->hub_loc, &state->remote_addr);
  if (!succ) {
    printf("Invalid IP String %s, exiting\n", ctx->hub_loc);
    return 1;
  }

  // Create lwip mqtt client
  state->mqtt_client = mqtt_client_new();
  state->counter = 0;

  if (state->mqtt_client == NULL) {
    printf("Failed to create MQTT client\n");
    return 1;
  }

  if (mqtt_connect(state) == ERR_OK) {
    absolute_time_t timeout = nil_time;
    int subscribed = 0;
    mqtt_set_inpub_callback(state->mqtt_client, mqtt_pub_start_cb,
                            mqtt_pub_data_cb, 0);
  }

  if (*ctx->prv_token != '\0') {
    DT_ERR_E err = provision(ctx);
    if (err != DT_ERR_OK) {
      printf("Failed to provision device\n");
      return err;
    }
  }
  ctx->mqtt_initd = true;

  return DT_ERR_OK;
}

DT_ERR_E mqtt_publish_handler(APP_CTX_T *ctx) {
  return breadcrumb(ctx);
}

DT_ERR_E mqtt_listen_handler(APP_CTX_T *ctx) {
  // Not listening for any topics for now
  return DT_ERR_OK;
}
