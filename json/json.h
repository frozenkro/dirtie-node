#ifndef JSON_H
#define JSON_H

typedef enum JsonErr {
  ERR_OK = 1,
  ERR_NEXT,
  ERR_SYN,
  ERR_STRLEN,
  ERR_STRCHR,
  ERR_ARRTYPE,
  ERR_DUPKEY,
  ERR_KEYLEN,
  ERR_KEYMISSING
} JSON_ERR_T_;

typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JSON_TYPE_T_;

typedef struct JsonVal JSON_VAL_T_;

typedef struct JsonKeyVal {
    char* key;
    JSON_VAL_T_* value;
    struct JsonKeyVal* next;
} JSON_KEY_VAL_T_;

typedef struct JsonArray {
    JSON_VAL_T_* value;
    struct JsonArray* next;
} JSON_ARRAY_T_;

typedef struct JsonVal {
    JSON_TYPE_T_ type;
    union {
        int boolean;
        double number;
        char* string;
        JSON_ARRAY_T_* array;
        JSON_KEY_VAL_T_* object;
    } value;
} JSON_VAL_T_;


int write(char *json, char *key, char *val);
int read(char *json, char *key, char *out_val);
JSON_ERR_T_ parse(char *json_str, int len, JSON_VAL_T_ *json_buf);

#endif
