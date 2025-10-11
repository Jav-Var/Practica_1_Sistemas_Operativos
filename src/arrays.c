#include "arrays.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

/* create arrays.dat and reserve header bytes */
int arrays_create(const char *path) {
    int fd = open(path, O_CREAT | O_TRUNC | O_RDWR, 0644);
    /* O_CREAT: creates the file if it doesn't exist.
     * O_TRUNC: if it exists, it truncates it to length 0 (deletes its content).
     * O_RDWR: opens it in read/write mode (necessary for pread/pwrite).
     */
    if (fd < 0) return -1;
    unsigned char header[ARRAYS_HEADER_SIZE];
    memset(header, 0, ARRAYS_HEADER_SIZE);
    if (safe_pwrite(fd, header, ARRAYS_HEADER_SIZE, 0) != (ssize_t)ARRAYS_HEADER_SIZE) {
        close(fd);
        return -1;
    }
    fsync(fd);
    close(fd);
    return 0;
}

int arrays_open(const char *path) {
    int fd = open(path, O_RDWR);
    return fd;
}

/* returns the size of a node with a key (string) of size key_len, and a list of offsets of size list_len */
size_t arrays_calc_node_size(uint16_t key_len, uint32_t list_len) {
    return sizeof(uint16_t)                  // key_len
         + (size_t)key_len                   // key bytes
         + sizeof(uint32_t)                  // list_len
         + (size_t)list_len * sizeof(uint64_t) // offsets[]
         + sizeof(uint64_t);                 // next_ptr
}

/* append node: serialize into a buffer then write at EOF */
offset_t arrays_append_node(int fd, const char *key, const offset_t *record_offsets_list, uint32_t list_len, offset_t next_ptr) {
    uint16_t key_len = (uint16_t)strlen(key);
    size_t node_size = arrays_calc_node_size(key_len, list_len);
    unsigned char *buf = malloc(node_size);
    if (!buf) return 0;
    size_t pos = 0;
    le_write_u16(buf + pos, key_len);
    pos += 2;
    memcpy(buf + pos, key, key_len);
    pos += key_len;
    le_write_u32(buf + pos, list_len);
    pos += 4;
    for (uint32_t i = 0; i < list_len; ++i) {
        le_write_u64(buf + pos, record_offsets_list[i]);
        pos += 8;
    }
    le_write_u64(buf + pos, next_ptr);
    pos += 8;
    /* compute offset at EOF */
    off_t new_off = lseek(fd, 0, SEEK_END);
    if (new_off == (off_t)-1) {
        free(buf);
        return 0;
    }
    /* ensure we don't write into header area 0: if file empty? arrays_create wrote header */
    if (new_off < ARRAYS_HEADER_SIZE) new_off = ARRAYS_HEADER_SIZE;
    if (safe_pwrite(fd, buf, node_size, new_off) != (ssize_t)node_size) {
        free(buf);
        return 0;
    }
    free(buf);
    /* optional fsync if durability required */
    return (offset_t)new_off;
}

int arrays_read_node_full(int fd, offset_t node_off, char **key_out, offset_t **offsets_out, uint32_t *list_len_out, offset_t *next_ptr_out) {
    /* read key_len first (2 bytes) */
    unsigned char tmp2[2];
    if (safe_pread(fd, tmp2, 2, node_off) != 2) return -1;
    uint16_t key_len = le_read_u16(tmp2);
    /* read next 4 + key_len bytes to get key and list_len */
    size_t head_read = (size_t)key_len + 4;
    unsigned char *buf = malloc(head_read);
    if (!buf) return -1;
    if (safe_pread(fd, buf, head_read, node_off + 2) != (ssize_t)head_read) {
        free(buf); return -1;
    }
    char *key = malloc(key_len + 1);
    if (!key) { free(buf); return -1; }
    memcpy(key, buf, key_len); key[key_len] = '\0';
    uint32_t list_len = le_read_u32(buf + key_len);
    free(buf);
    /* read offsets and next_ptr */
    size_t rest = (size_t)list_len * 8 + 8;
    unsigned char *restbuf = malloc(rest);
    if (!restbuf) { free(key); return -1; }
    if (safe_pread(fd, restbuf, rest, node_off + 2 + key_len + 4) != (ssize_t)rest) {
        free(key); free(restbuf); return -1;
    }
    offset_t *offsets = NULL;
    if (list_len > 0) {
        offsets = malloc(sizeof(offset_t) * list_len);
        if (!offsets) { free(key); free(restbuf); return -1; }
        for (uint32_t i = 0; i < list_len; ++i) {
            offsets[i] = le_read_u64(restbuf + i * 8);
        }
    }
    offset_t next = le_read_u64(restbuf + (size_t)list_len * 8);
    free(restbuf);
    *key_out = key;
    *offsets_out = offsets;
    *list_len_out = list_len;
    *next_ptr_out = next;
    return 0;
}
