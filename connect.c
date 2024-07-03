#include "connect.h"
#include "dirtie_config.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"
#include "lwip/ip4_addr.h"
#include "lwip/ip_addr.h"
#include "pico/cyw43_arch.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "pico/types.h"
#include "usb_cfg.h"
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 2048

#define MQTT_PORT 1883

typedef struct MQTT_CLIENT_T_ {
  ip_addr_t remote_addr;
  mqtt_client_t *mqtt_client;
  u32_t received;
  u32_t counter;
  u32_t reconnect;
} MQTT_CLIENT_T;

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
  char buffer[128];

  sprintf(buffer, "{\"message\":\"test dirtie node %d / %d \"}",
          state->received, state->counter);

  err_t err;
  u8_t qos =
      0; /* 0 1 or 2, see MQTT specification.  AWS IoT does not support QoS 2 */
  u8_t retain = 0;

  cyw43_arch_lwip_begin();
  err = mqtt_publish(state->mqtt_client, "pico_w/test", buffer, strlen(buffer),
                     qos, retain, mqtt_pub_request_cb, state);

  cyw43_arch_lwip_end();

  if (err != ERR_OK) {
    printf("Publish err: %d\n", err);
  }

  return err;
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

  // Skipping TLS for initial test btw
  //

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
    printf("Failed to create MQTT client");
    return 1;
  }

  if (mqtt_test_connect(state) == ERR_OK) {
    absolute_time_t timeout = nil_time;
    bool subscribed = false;
    mqtt_set_inpub_callback(state->mqtt_client, mqtt_pub_start_cb,
                            mqtt_pub_data_cb, 0);

    while (true) {
      cyw43_arch_poll();
      absolute_time_t now = get_absolute_time();
      if (is_nil_time(timeout) || absolute_time_diff_us(now, timeout) <= 0) {
        if (mqtt_client_is_connected(state->mqtt_client)) {
          cyw43_arch_lwip_begin();

          if (!subscribed) {
            mqtt_sub_unsub(state->mqtt_client, "pico_w/recv", 0,
                           mqtt_sub_request_cb, 0, 1);
            subscribed = true;
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
    }
  }

  return 0;
}

int mqtt_test(char ssid[], char password[], char ip[]) {
  // put in station mode because we are making connections from device
  if (!WIFI_CONFIGURED) {
    printf("wifi credentials not yet configured\n");
    return 1;
  }
  cyw43_arch_enable_sta_mode();

  printf("connecting to %s\n", ssid);
  if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                         CYW43_AUTH_WPA2_AES_PSK, 30000)) {
    printf("failed to connect.\n");
    return 1;
  } else {
    printf("Connected.\n");
  }

  MQTT_CLIENT_T *state = mqtt_client_init();

  // assign ip to state based on string
  int succ = ip4addr_aton(ip, &state->remote_addr);
  if (!succ) {
    printf("Invalid IP String %s, exiting", ip);
    return 1;
  }

  if (mqtt_run_test(state)) {
    printf("MQTT Test Failed, exiting");
    return 1;
  }

  return 0;
}
