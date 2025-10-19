
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

