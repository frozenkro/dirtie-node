#include "util.h"
#include "../state/state.h"
#include "../logger/logger.h"

#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"

#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/err.h"

/* ---- Struct definition ---- */
typedef struct NTP_T_ {
  ip_addr_t ntp_server_address;
  bool dns_request_sent;
  struct udp_pcb *ntp_pcb;
  absolute_time_t ntp_next_poll;
  alarm_id_t ntp_resend_alarm;

  /* Cached wall-clock epoch (seconds since 1970) and corresponding HW boot ms */
  uint64_t epoch_secs;
  uint64_t hw_ref_ms;
  bool synced;
} NTP_T;

/* ---- Constants ---- */
#define NTP_SERVER       "pool.ntp.org"
#define NTP_MSG_LEN      48
#define NTP_PORT         123
#define NTP_DELTA        2208988800ull  /* seconds between 1900 and 1970 */
#define NTP_POLL_MS      (30 * 1000)    /* re-sync every 30 s */
#define NTP_RESEND_MS    (10 * 1000)    /* give-up if no response in 10 s */

/* Forward declarations for callbacks */
static int64_t ntp_failed_handler(alarm_id_t id, void *user_data);
static void    ntp_request(NTP_T *state);
static void    ntp_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg);
static void    ntp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

/* ---- Helpers ---- */

static void ntp_request(NTP_T *state) {
  cyw43_arch_lwip_begin();
  struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
  if (!p) {
    cyw43_arch_lwip_end();
    printf("ntp: pbuf_alloc failed\n");
    return;
  }
  uint8_t *req = (uint8_t *)p->payload;
  memset(req, 0, NTP_MSG_LEN);
  req[0] = 0x1b;                      /* NTP client request byte */
  udp_sendto(state->ntp_pcb, p, &state->ntp_server_address, NTP_PORT);
  pbuf_free(p);
  cyw43_arch_lwip_end();
}

/* Called when a sync attempt completes (success or failure) */
static void ntp_sync_done(NTP_T *state, bool success) {
  if (state->ntp_resend_alarm > 0) {
    cancel_alarm(state->ntp_resend_alarm);
    state->ntp_resend_alarm = 0;
  }
  state->dns_request_sent = false;
  state->ntp_next_poll = make_timeout_time_ms(NTP_POLL_MS);
}

static int64_t ntp_failed_handler(alarm_id_t id, void *user_data) {
  (void)id;
  NTP_T *state = (NTP_T *)user_data;
  printf("ntp: request timed out\n");
  ntp_sync_done(state, false);
  return 0;
}

static void ntp_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
  (void)hostname;
  NTP_T *state = (NTP_T *)arg;
  if (ipaddr) {
    state->ntp_server_address = *ipaddr;
    printf("ntp: resolved %s\n", ipaddr_ntoa(ipaddr));
    ntp_request(state);
  } else {
    printf("ntp: DNS lookup failed\n");
    ntp_sync_done(state, false);
  }
}

static void ntp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
  (void)pcb;
  NTP_T *state = (NTP_T *)arg;

  uint8_t mode    = pbuf_get_at(p, 0) & 0x7;
  uint8_t stratum = pbuf_get_at(p, 1);

  /* Validate response */
  if (ip_addr_cmp(addr, &state->ntp_server_address) &&
      port == NTP_PORT &&
      p->tot_len == NTP_MSG_LEN &&
      mode == 0x4 && stratum != 0) {

    uint8_t seconds_buf[4] = {0};
    pbuf_copy_partial(p, seconds_buf, sizeof(seconds_buf), 40);
    uint32_t seconds_since_1900 = ((uint32_t)seconds_buf[0] << 24) |
                                  ((uint32_t)seconds_buf[1] << 16) |
                                  ((uint32_t)seconds_buf[2] <<  8) |
                                  ((uint32_t)seconds_buf[3]      );
    uint32_t seconds_since_1970 = seconds_since_1900 - NTP_DELTA;

    time_t epoch = (time_t)seconds_since_1970;
    struct tm *utc = gmtime(&epoch);
    printf("ntp: sync ok  %02d/%02d/%04d %02d:%02d:%02d UTC\n",
           utc->tm_mday, utc->tm_mon + 1, utc->tm_year + 1900,
           utc->tm_hour, utc->tm_min, utc->tm_sec);

    /* Cache the epoch + the HW time at which it was valid */
    state->epoch_secs  = seconds_since_1970;
    state->hw_ref_ms   = to_ms_since_boot(get_absolute_time());
    state->synced      = true;

    ntp_sync_done(state, true);
  } else {
    printf("ntp: invalid response (mode=%u stratum=%u len=%d)\n", mode, stratum, p->tot_len);
    ntp_sync_done(state, false);
  }
  pbuf_free(p);
}

/* ---- Public API ---- */

NTP_T* ntp_init(void) {
  NTP_T *state = (NTP_T *)calloc(1, sizeof(NTP_T));
  if (!state) {
    printf("ntp: calloc failed\n");
    return NULL;
  }
  state->ntp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
  if (!state->ntp_pcb) {
    printf("ntp: udp_new_ip_type failed\n");
    free(state);
    return NULL;
  }
  udp_recv(state->ntp_pcb, ntp_recv, state);
  state->ntp_next_poll = get_absolute_time(); /* trigger immediately on first poll */
  return state;
}

void ntp_deinit(NTP_T *state) {
  if (!state) return;
  if (state->ntp_resend_alarm > 0) {
    cancel_alarm(state->ntp_resend_alarm);
  }
  if (state->ntp_pcb) {
    udp_remove(state->ntp_pcb);
  }
  free(state);
}

/* Non-blocking periodic tick – call from your main loop */
void ntp_poll(NTP_T *state) {
  if (!state) return;

  if (absolute_time_diff_us(get_absolute_time(), state->ntp_next_poll) < 0 &&
      !state->dns_request_sent) {

    state->ntp_resend_alarm = add_alarm_in_ms(NTP_RESEND_MS, ntp_failed_handler, state, true);

    cyw43_arch_lwip_begin();
    int err = dns_gethostbyname(NTP_SERVER, &state->ntp_server_address, ntp_dns_found, state);
    cyw43_arch_lwip_end();

    state->dns_request_sent = true;

    if (err == ERR_OK) {
      ntp_request(state);               /* DNS was already cached */
    } else if (err != ERR_INPROGRESS) { /* INPROGRESS = callback coming */
      printf("ntp: dns request failed (%d)\n", err);
      ntp_sync_done(state, false);
    }
  }
}

/* Compute wall-clock epoch ms */
DT_ERR_E get_current_epoch(APP_CTX_T *ctx, uint64_t *result) {
  if (!ctx || !ctx->ntp_state) {
    return DT_ERR_OFFLINE;
  }

  if (!ctx->ntp_state->synced) {
    return DT_ERR_OFFLINE;
  }

  uint64_t now_ms = to_ms_since_boot(get_absolute_time());
  uint64_t delta_ms = now_ms - ctx->ntp_state->hw_ref_ms;
  *result = (ctx->ntp_state->epoch_secs * 1000ULL) + delta_ms;
  return DT_ERR_OK;
}

/* ---- State-machine handler ---- */

DT_ERR_E ntp_sync_handler(APP_CTX_T *ctx) {
  if (!ctx->ntp_state) {
    ctx->ntp_state = ntp_init();
    if (!ctx->ntp_state) {
      dlog(ctx, LOG_ERR, "ntp_init failed\n");
      return DT_ERR_MALLOC;
    }
  }

  ntp_poll(ctx->ntp_state);

  NTP_T *ntp = (NTP_T *)ctx->ntp_state;
  if (ntp->synced) {
    ctx->ntp_syncd = true;
  }

  return DT_ERR_OK;
}
