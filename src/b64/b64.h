/**
 * `b64.h' - b64
 *
 * copyright (c) 2014 joseph werle
 */
// Based on b64 by Joseph Werle (https://github.com/jwerle/b64.c)
// Original copyright (c) 2014 Joseph Werle
// Modified by Kaleb Olson 2026: Modify imports to work in this project, remove unused macros/funcs

#include <stddef.h>
#ifndef B64_H
#define B64_H 1

typedef struct b64_buffer {
    char * ptr;
    size_t bufc;
} b64_buffer_t;

#ifndef b64_malloc
#  define b64_malloc(ptr) malloc(ptr)
#endif
#ifndef b64_realloc
#  define b64_realloc(ptr, size) realloc(ptr, size)
#endif

 // How much memory to allocate per buffer
#ifndef B64_BUFFER_SIZE
#  define B64_BUFFER_SIZE		(1024 * 64) // 64K
#endif

/**
 * Base64 index table.
 */

static const char b64_table[] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '+', '/'
};

/**
 * Encode `unsigned char *' source with `size_t' size.
 * Returns a `char *' base64 encoded string.
 */

char *
b64_encode (const unsigned char *, size_t);

#endif
