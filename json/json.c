#include "json/json.h"
#include <cstdio>
#include <stdio.h>
#include <string.h>

const char *JSON_TRUE_S = "true";
const char *JSON_FALSE_S = "false";
const char *JSON_NULL_S = "null";

typedef struct parsing_state {
  char *cursor;
  JSON_ERR_T_ err;
} PARSING_STATE_T_;

typedef JSON_RESULT_T_ (*walk_callback)(PARSING_STATE_T_*);
JSON_RESULT_T_ walk_key_val(PARSING_STATE_T_ *state);
JSON_RESULT_T_ walk_arr(PARSING_STATE_T_ *state);
JSON_RESULT_T_ walk_str(PARSING_STATE_T_ *state);
JSON_RESULT_T_ walk_bool(PARSING_STATE_T_ *state);
JSON_RESULT_T_ walk_num(PARSING_STATE_T_ *state);
JSON_RESULT_T_ walk_null(PARSING_STATE_T_ *state);
const walk_callback WALK_VAL_CALLBACKS[6 * sizeof(walk_callback)] = { walk_key_val, walk_arr, walk_str, walk_bool, walk_num };

JSON_ERR_T_ parse(char *json_str, int len, JSON_VAL_T_ *json_buf) {

  return ERR_OK;
}

void next_ch(PARSING_STATE_T_ *state) {
  while (*state->cursor == ' ' || *state->cursor == '\n') state->cursor++;
}

JSON_RESULT_T_ walk_key_val(PARSING_STATE_T_ *state) {
  if (*state->cursor != '{') {
    struct JsonResult retval = { ERR_SYN, NULL };
    return retval;
  }
}

JSON_RESULT_T_ walk_arr(PARSING_STATE_T_ *state) {
  if (*state-> cursor != '[') {
    struct JsonResult retval = { ERR_SYN, NULL };
    return retval;
  }
}

JSON_RESULT_T_ walk_str(PARSING_STATE_T_ *state) {
  if (*state->cursor != '"') {
    struct JsonResult retval = { ERR_SYN, NULL };
    return retval;
  }
}

JSON_RESULT_T_ walk_bool(PARSING_STATE_T_ *state) {
  // if error, reset state cursor to init
  char *init_cursor = state->cursor;
  if (strncmp(state->cursor, JSON_TRUE_S, strlen(JSON_TRUE_S)) == 0) {
    // allocate struct for val
  }

}

JSON_RESULT_T_ walk_num(PARSING_STATE_T_ *state) {
}
JSON_RESULT_T_ walk_null(PARSING_STATE_T_ *state) {
}

JSON_RESULT_T_ walk_val(PARSING_STATE_T_ *state) {
  JSON_RESULT_T_ res;
  for (int i = 0; i < 6; i++) {
    next_ch(state);

    int index = i * sizeof(walk_callback);
    res = WALK_VAL_CALLBACKS[i](state);

    if (res.err == ERR_OK || res.err != ERR_SYN) {
      return res;
    }
  }

  return res;
}

