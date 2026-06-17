#include "logger.h"
#include "dt_err.h"
#include "state/state.h"
#include <pico/time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DT_ERR_E dlog(APP_CTX_T *ctx, LOG_LVL_E lvl, char* fmesg, ...) {
  va_list args;
  va_start(args, fmesg);

  // Could do this on stack but its on an rp2040 so playing it safe
  char *printbuf = calloc(MAX_LOG_LEN, 1);
  if (printbuf == NULL) {
    return DT_ERR_MALLOC;
  }

  // Start by getting hw clock seconds (we may not have network connection yet for epoch)
  // Recalc on publish based on hw clock at the time
  uint64_t ms = to_ms_since_boot(get_absolute_time());
  char *lvlstr = lvl == LOG_INFO ? "INFO" : "ERROR";

  char *prefix = "[%013llu] [%s]: ";
  int pxlen = snprintf(NULL, 0, prefix, ms, lvlstr);
  snprintf(printbuf, pxlen + 1, prefix, ms, lvlstr);

  int remaining = MAX_LOG_LEN - pxlen;
  int needed = vsnprintf(printbuf + pxlen, MAX_LOG_LEN - pxlen, fmesg, args);
  va_end(args);

  if (needed > remaining) {
    const char *truncmark = "...[TCD]";
    strncpy(printbuf + MAX_LOG_LEN - strlen(truncmark) - 1,
        truncmark, strlen(truncmark) + 1);
  }

  // stdout for debuggin
  printf("%s\n", printbuf);

  // Add log to log dump
  int maxlogentries = sizeof(ctx->logdump) / sizeof(char*);
  for (int i = 0; i < maxlogentries; ++i) {

    if (ctx->logdump[i] == NULL) {
      ctx->logdump[i] = printbuf;
      return DT_ERR_OK;
    }

    // flag cleanup early if log entries are at 50% capacity
    if (i >= maxlogentries / 2) {
      ctx->dump_logs = 1;
    }
  }

  // Data loss - too many logs in one cycle
  free(printbuf);
  return DT_ERR_2MANYLOGS;
}

void free_logs(APP_CTX_T *ctx) {
  
  for (int i = 0; i < sizeof(ctx->logdump) / sizeof(char*); ++i) {
    if (ctx->logdump[i] == NULL) {
      // break is also fine but hey its 16 loops and idk maybe one log is broken
      continue; 
    }

    free(ctx->logdump[i]);
  }
}

void log_transform_ts(char* msg, uint64_t now, uint64_t hwnow) {
  // Extract hw timestamp from log msg
  char* ts_start = strchr(msg, '[');
  if (ts_start == NULL) return;
  ts_start++;

  char* ts_end;
  uint64_t ts = strtoull(ts_start, &ts_end, 10);

  uint64_t delta = hwnow - ts;
  uint64_t epochts = now - delta;

  char buf[14];
  snprintf(buf, sizeof(buf), "%013llu", epochts);

  memcpy(ts_start, buf, sizeof(buf) - 1);
}
