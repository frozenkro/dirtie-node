#include <cyw43_ll.h>
#include <lwip/err.h>
#include <stdbool.h>
#include <string.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "dhcpserver.h"
#include "dt_globals.h"
#include "access_point/host.h"

#define TCP_PORT 80
#define POLL_TIME_S 5
#define HTTP_POST "POST"
#define WIFI_CREDENTIALS_ENDPOINT "/wifi-setup"
#define MAX_CONTENT_LENGTH 1024

int *HOSTING = 0;

typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    bool complete;
    ip_addr_t gw;
} TCP_SERVER_T;

typedef struct TCP_CONNECT_STATE_T_ {
    struct tcp_pcb *pcb;
    int sent_len;
    char headers[128];
    char result[256];
    int header_len;
    int result_len;
    ip_addr_t *gw;
} TCP_CONNECT_STATE_T;

static err_t tcp_close_client_connection(TCP_CONNECT_STATE_T *con_state, struct tcp_pcb *client_pcb, err_t close_err) {
    if (client_pcb) {
        assert(con_state && con_state->pcb == client_pcb);
        tcp_arg(client_pcb, NULL);
        tcp_poll(client_pcb, NULL, 0);
        tcp_sent(client_pcb, NULL);
        tcp_recv(client_pcb, NULL);
        tcp_err(client_pcb, NULL);
        err_t err = tcp_close(client_pcb);
        if (err != ERR_OK) {
            printf("close failed %d, calling abort\n", err);
            tcp_abort(client_pcb);
            close_err = ERR_ABRT;
        }
        if (con_state) {
            free(con_state);
        }
    }
    return close_err;
}

static void tcp_server_close(TCP_SERVER_T *state) {
    if (state->server_pcb) {
        tcp_arg(state->server_pcb, NULL);
        tcp_close(state->server_pcb);
        state->server_pcb = NULL;
    }
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    printf("tcp_server_sent %u\n", len);
    con_state->sent_len += len;
    if (con_state->sent_len >= con_state->header_len + con_state->result_len) {
        printf("all done\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    return ERR_OK;
}

static bool parse_wifi_credentials(char *json) {
  const char* ssid_query = "\"ssid\":\"";
  int ssid_skip = strlen(ssid_query);
  const char* ssid_start = strstr(json, ssid_query);

  const char* pw_query = "\"password\":\"";
  int pw_skip = strlen(pw_query);
  const char* pw_start;
  if (!ssid_start || !pw_start) {
    return false;
  }

  ssid_start += ssid_skip;
  pw_start += pw_skip;

  const char* ssid_end = strchr(ssid_start, '"');
  const char* pw_end = strchr(pw_start, '"');

  if (!ssid_end || ssid_end - ssid_start > 64 || !pw_end || pw_end - pw_start > 64) {
    return false;
  }

  memcpy(WIFI_SSID, ssid_start, ssid_end - ssid_start);
  WIFI_SSID[ssid_end - ssid_start] = '\0';
  memcpy(WIFI_PASSWORD, pw_start, pw_end - pw_start);
  WIFI_PASSWORD[pw_end - pw_start] = '\0';

  WIFI_CONFIGURED = 1;
  return true;
}

err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (!p) {
        printf("connection closed\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    assert(con_state && con_state->pcb == pcb);
    if (p->tot_len > 0) {
        // Copy the request into the buffer
        pbuf_copy_partial(p, con_state->headers, p->tot_len > sizeof(con_state->headers) - 1 ? sizeof(con_state->headers) - 1 : p->tot_len, 0);

        // Handle POST request
        if (strncmp(HTTP_POST, con_state->headers, sizeof(HTTP_POST) - 1) == 0) {
            char *path = con_state->headers + sizeof(HTTP_POST); // skip "POST "
            while (*path == ' ') path++; //skip all leading space
                                         
            if (strncmp(path, WIFI_CREDENTIALS_ENDPOINT, sizeof(WIFI_CREDENTIALS_ENDPOINT) - 1) == 0) {
              // get the request body (json) start point - after two line breaks
              char *json_start = strstr(con_state->headers, "\r\n\r\n");
              if (json_start) {
                  //skip rnrn
                  json_start += 4;

                  if (parse_wifi_credentials(json_start)) {
                    const char* response = "HTTP/1.1 200 OK\r\n\r\n";

                    err_t err = tcp_write(pcb, response, strlen(response), 0);

                    if (!err) {
                      err = ERR_OK;
                    }
                    return tcp_close_client_connection(con_state, pcb, err);
                  } else {
                    const char *body = "{\"error\":\"invalid_format\"}";
                    int body_len = strlen(body);

                    char response[256];
                    snprintf(response, sizeof(response),
                      "HTTP/1.1 400 Bad Request\nContent-Length: %d\nContent-Type: application/json\r\n\r\n%s",
                      body_len, body);
                    err_t err = tcp_write(pcb, response, strlen(response), 0);

                    if (!err) {
                      err = ERR_OK;
                    }
                    return tcp_close_client_connection(con_state, pcb, err);
                  }
              }

            }

        }
        tcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);
    return ERR_OK;
}

static err_t tcp_server_poll(void *arg, struct tcp_pcb *pcb) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    printf("tcp_server_poll_fn\n");
    return tcp_close_client_connection(con_state, pcb, ERR_OK); // Just disconnect clent?
}

static void tcp_server_err(void *arg, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (err != ERR_ABRT) {
        printf("tcp_client_err_fn %d\n", err);
        tcp_close_client_connection(con_state, con_state->pcb, err);
    }
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        printf("failure in accept\n");
        return ERR_VAL;
    }
    printf("client connected\n");

    // Create the state for the connection
    TCP_CONNECT_STATE_T *con_state = calloc(1, sizeof(TCP_CONNECT_STATE_T));
    if (!con_state) {
        printf("failed to allocate connect state\n");
        return ERR_MEM;
    }
    con_state->pcb = client_pcb; // for checking
    con_state->gw = &state->gw;

    // setup connection to client
    tcp_arg(client_pcb, con_state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);

    return ERR_OK;
}

static int tcp_server_open(void *arg, const char *ap_name) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    printf("starting server on port %d\n", TCP_PORT);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        printf("failed to create pcb\n");
        return 1;
    }

    err_t err = tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
    if (err) {
        printf("failed to bind to port %d\n",TCP_PORT);
        return 1;
    }

    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        printf("failed to listen\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return 1;
    }

    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    return 0;
}


int host_provisioning_ap() {

    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        printf("failed to allocate state\n");
        return 1;
    }

    const char *ap_name = "dirtie";
    const char *password = NULL;
    cyw43_arch_enable_ap_mode(ap_name, password, CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t mask;
    IP4_ADDR(ip_2_ip4(&state->gw), 192, 168, 4, 1);
    IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 2);

    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &state->gw, &mask);

    if (!tcp_server_open(state, ap_name)) {
        printf("failed to open server\n");
        return 1;
    }

    state->complete = false;
    while(!state->complete) {
      // wait for connection from device and provisioning info to be sent 
      // then continue
      sleep_ms(500);
    }

    return 0;
}
