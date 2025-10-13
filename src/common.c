
#define _XOPEN_SOURCE 500

#include "common.h"
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

ssize_t safe_pread(int fd, void *buf, size_t count, off_t offset) {
    ssize_t total = 0;
    char *p = (char*)buf;
    while (total < (ssize_t)count) {
        ssize_t r = pread(fd, p + total, count - total, offset + total);
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) break;
        total += r;
    }
    return total;
}

ssize_t safe_pwrite(int fd, const void *buf, size_t count, off_t offset) {
    ssize_t total = 0;
    const char *p = (const char*)buf;
    while (total < (ssize_t)count) {
        ssize_t w = pwrite(fd, p + total, count - total, offset + total);
        if (w < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        total += w;
    }
    return total;
}

/* little-endian conversions (assume target can be either endian) */
uint16_t le_read_u16(const void *buf) {
    const unsigned char *b = buf;
    return (uint16_t)b[0] | ((uint16_t)b[1] << 8);
}
uint32_t le_read_u32(const void *buf) {
    const unsigned char *b = buf;
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}
uint64_t le_read_u64(const void *buf) {
    const unsigned char *b = buf;
    uint64_t v = 0;
    v |= (uint64_t)b[0];
    v |= (uint64_t)b[1] << 8;
    v |= (uint64_t)b[2] << 16;
    v |= (uint64_t)b[3] << 24;
    v |= (uint64_t)b[4] << 32;
    v |= (uint64_t)b[5] << 40;
    v |= (uint64_t)b[6] << 48;
    v |= (uint64_t)b[7] << 56;
    return v;
}
void le_write_u16(void *buf, uint16_t v) {
    unsigned char *b = buf;
    b[0] = v & 0xff;
    b[1] = (v >> 8) & 0xff;
}
void le_write_u32(void *buf, uint32_t v) {
    unsigned char *b = buf;
    b[0] = v & 0xff;
    b[1] = (v >> 8) & 0xff;
    b[2] = (v >> 16) & 0xff;
    b[3] = (v >> 24) & 0xff;
}
void le_write_u64(void *buf, uint64_t v) {
    unsigned char *b = buf;
    b[0] = v & 0xff;
    b[1] = (v >> 8) & 0xff;
    b[2] = (v >> 16) & 0xff;
    b[3] = (v >> 24) & 0xff;
    b[4] = (v >> 32) & 0xff;
    b[5] = (v >> 40) & 0xff;
    b[6] = (v >> 48) & 0xff;
    b[7] = (v >> 56) & 0xff;
}

char *normalize_string(const char *s) {
    if (!s) {
        char *empty = malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    size_t in_len = strlen(s);
    /* worst-case buffer: same length + 1 */
    char *out = malloc(in_len + 1);
    if (!out) return NULL;

    size_t in_i = 0, out_j = 0;
    int last_was_space = 0;

    for (in_i = 0; in_i < in_len; ++in_i) {
        unsigned char c = (unsigned char)s[in_i];

        /* ASCII letters/digits */
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            /* convert to lowercase for letters */
            unsigned char lc = (unsigned char)tolower(c);
            out[out_j++] = (char)lc;
            last_was_space = 0;
        } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            /* collapse spaces: only insert a single space if previous wasn't space and not at start */
            if (!last_was_space && out_j > 0) {
                out[out_j++] = ' ';
                last_was_space = 1;
            }
        } else {
            /* skip everything else (punctuation, non-ASCII bytes, symbols) */
        }
    }

    /* trim trailing space */
    if (out_j > 0 && out[out_j - 1] == ' ') out_j--;

    out[out_j] = '\0';

    char *shrink = realloc(out, out_j + 1);
    if (shrink) return shrink;
    return out;
}

/* normalized_strcmp: compares normalized versions of a and b.
 * returns same semantics as strcmp.
 * handles NULL pointers (treat as empty).
 */
int normalized_strcmp(const char *a, const char *b) {
    char *na = normalize_string(a ? a : "");
    char *nb = normalize_string(b ? b : "");
    if (!na || !nb) {
        if (na) free(na);
        if (nb) free(nb);
        return strcmp(a ? a : "", b ? b : "");
    }
    int r = strcmp(na, nb);
    free(na);
    free(nb);
    return r;
}