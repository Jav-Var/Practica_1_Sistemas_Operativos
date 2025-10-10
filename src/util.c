#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

uint64_t next_pow2(uint64_t v) {
    if (v == 0) return 1;
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    v++;
    return v;
}

char *trim_inplace(char *s) {
    if (!s) return s;
    char *end;
    while (isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

char *xstrdup(const char *s) {
    if (!s) return NULL;
    char *d = malloc(strlen(s) + 1);
    if (!d) return NULL;
    strcpy(d, s);
    return d;
}
