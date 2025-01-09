#include "json/json.h"
#include <stdio.h>
#include <string.h>

/* JSON_ERR_T write(char *json, char *key, char *val) { */

/* } */

/* JSON_ERR_T read(char *json, char *key, char *out_val) { */
/*   const int keylen = strlen(key); */
/*   const int querylen = keylen + (strlen("\"") * 2); */
/*   char query[querylen]; */
/*   snprintf(query, querylen, "\"%s\"", key); */

/*   char *value_start = strstr(json, query); */
/*   value_start += querylen; */
/*   while */ 
/* } */

JSON_ERR_T_ parse(char *json_str, int len, JSON_VAL_T_ *json_buf) {

  return ERR_OK;
}
