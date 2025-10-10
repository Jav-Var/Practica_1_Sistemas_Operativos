#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

uint64_t next_pow2(uint64_t v);

/* trim leading/trailing whitespace in-place and return pointer */
char *trim_inplace(char *s);

/* safe strdup */
char *xstrdup(const char *s);

#endif // UTIL_H
