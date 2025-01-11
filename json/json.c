#include "json/json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *JSON_TRUE_S = "true";
const char *JSON_FALSE_S = "false";
const char *JSON_NULL_S = "null";

typedef struct parsing_state {
  char *cursor;
  JSON_ERR_T_ err;
} PARSING_STATE_T_;

typedef struct JsonResult {
  JSON_ERR_T_ err;
  JSON_VAL_T_ *val;
} JSON_RESULT_T_;

typedef JSON_RESULT_T_ (*walk_callback)(PARSING_STATE_T_*);
JSON_RESULT_T_ walk_key_val(PARSING_STATE_T_ *state);
JSON_RESULT_T_ walk_arr(PARSING_STATE_T_ *state);
JSON_RESULT_T_ walk_str(PARSING_STATE_T_ *state);
JSON_RESULT_T_ walk_keyword(PARSING_STATE_T_ *state);
const walk_callback WALK_VAL_CALLBACKS[4 * sizeof(walk_callback)] = { walk_key_val, walk_arr, walk_str, walk_keyword };

JSON_RESULT_T_ err_next() {
  return (JSON_RESULT_T_){ ERR_NEXT, NULL };
}
JSON_RESULT_T_ err_syn() {
  return (JSON_RESULT_T_){ ERR_SYN, NULL };
}

JSON_ERR_T_ parse(char *json_str, int len, JSON_VAL_T_ *json_buf) {

  return ERR_OK;
}

void next_ch(PARSING_STATE_T_ *state) {
  while (*state->cursor == ' ' || *state->cursor == '\n') state->cursor++;
}

JSON_RESULT_T_ walk_key_val(PARSING_STATE_T_ *state) {
  if (*state->cursor != '{') {
    return err_next();
  }
}

JSON_RESULT_T_ walk_arr(PARSING_STATE_T_ *state) {
  if (*state-> cursor != '[') {
    return err_next();
  }

  
}

// Skipping some string escapes for now
JSON_RESULT_T_ walk_str(PARSING_STATE_T_ *state) {
  if (*state->cursor != '"') {
    return err_next();
  }

  char *val = malloc(255);
  int esc = 0;

  for (int i = 1; i < 256; i++) {
    char ch = state->cursor[i];
    if (ch == '"') {
      state->cursor += i + 1;
      JSON_VAL_T_ *jval = malloc(sizeof(JSON_VAL_T_));
      return (JSON_RESULT_T_){ ERR_OK, jval };
    }

    if (esc) {
      if (ch == '\\') {
        val[i-1] = ch;
      }
      else if (ch == '/') {
        val[i-1] = '/';
      }
      else if (ch == '"') {
        val[i-1] = '\"';
      }
      else if (ch == 'f') {
        val[i-1] = '\f';
      }
      else if (ch == 'b') {
        val[i-1] = '\b';
      }
      else if (ch == 'n') {
        val[i-1] = '\n';
      }
      else if (ch == 'r') {
        val[i-1] = '\r';
      }
      else if (ch == 't') {
        val[i-1] = '\t';
      }
      else if (ch == 'r') {
        val[i-1] = '\r';
      } 
      else {
        return (JSON_RESULT_T_){ ERR_STRCHR, NULL };
      }
      
      esc = 0;
    } else if (ch == '\\') {
      esc = 1;
    } else {
      val[i-1] = ch;
    }
  }

  free(val);
  return (JSON_RESULT_T_){ ERR_STRLEN, NULL };
}

int parse_keyword(PARSING_STATE_T_ *state, const char *keyword) {
  if (strncmp(state->cursor, keyword, strlen(keyword)) == 0) {

    const char nc = state->cursor[strlen(keyword)];
    if (nc == ' ' || nc == '\n' || nc == '\r' 
        || nc == ',' || nc == ']' || nc == '}') {
      return 1;
    }
  }
  return 0;
}

JSON_RESULT_T_ walk_bool(PARSING_STATE_T_ *state) {
  if (parse_keyword(state, JSON_TRUE_S)) {
    state->cursor += strlen(JSON_TRUE_S) + 1;
    JSON_VAL_T_ *jval = malloc(sizeof(JSON_VAL_T_));
    jval->type = JSON_BOOL;
    jval->value.boolean = 1;
    return (JSON_RESULT_T_){ ERR_OK, jval };
  }

  if (parse_keyword(state, JSON_FALSE_S)) {
    state->cursor += strlen(JSON_FALSE_S) + 1;
    JSON_VAL_T_ *jval = malloc(sizeof(JSON_VAL_T_));
    jval->type = JSON_BOOL;
    jval->value.boolean = 0;
    return (JSON_RESULT_T_){ ERR_OK, jval };
  }

  return err_next();
}

JSON_RESULT_T_ walk_null(PARSING_STATE_T_ *state) {
  if (parse_keyword(state, JSON_NULL_S)) {
    state->cursor += strlen(JSON_NULL_S) + 1;
    JSON_VAL_T_ *jval = malloc(sizeof(JSON_VAL_T_));
    jval->type = JSON_NULL;
    return (JSON_RESULT_T_){ ERR_OK, jval };
  }

  return err_next();
}

JSON_RESULT_T_ walk_keyword(PARSING_STATE_T_ *state) {
  if (*state->cursor != *JSON_TRUE_S && 
      *state->cursor != *JSON_FALSE_S && 
      *state->cursor != *JSON_NULL_S) {
    return err_next();
  }
  JSON_RESULT_T_ res = walk_bool(state);
  if (res.err == ERR_NEXT) {
    res = walk_null(state);
  }
  return res;
}

int valid_digit(char c) {
  return (c == '.' || (c > '0' && c < '9'));
}

JSON_RESULT_T_ walk_num(PARSING_STATE_T_ *state) {
  if (*state->cursor != '.' && 
      (*state->cursor < '0' || *state->cursor > '9')) {
    return err_next();
  }

  char *term = state->cursor;
  char *dec = NULL;
  while (valid_digit(*term)) {
    if (*term == '.') {
      if (dec != NULL) {
        return err_syn();
      }
      dec = term;
    }
    term++;
  }
  
  const char *intlen = dec == NULL ? term : dec;
  double result = 0.0;

  //handle decimal
  if (dec != NULL) {
    const int len = term - dec;
    for (int i = 0; i < len; i++) {
      char ch = state->cursor[i];

      double num = ch - '0';
      for (int j = 0; j <= i; j++) {
        num = num * 0.1;
      }

      result += num;
    }
  }

  int intres = 0;
  for (int i = *intlen; i >= *state->cursor; i--) {
    char ch = state->cursor[i];

    int num = ch - '0';
    for (int j = 0; j < *intlen - i; j++) {
      num = num * 10;
    }

    result += num;
  }

  state->cursor = term;
  JSON_VAL_T_ *jval = malloc(sizeof(JSON_VAL_T_));
  jval->type = JSON_NUMBER;
  jval->value.number = result;
  return (JSON_RESULT_T_){ ERR_OK, jval };
}

JSON_RESULT_T_ walk_val(PARSING_STATE_T_ *state) {
  JSON_RESULT_T_ res;
  for (int i = 0; i < 4; i++) {
    next_ch(state);

    int index = i * sizeof(walk_callback);
    res = WALK_VAL_CALLBACKS[i](state);

    if (res.err == ERR_OK || res.err != ERR_NEXT) {
      return res;
    }
  }

  return res;
}

