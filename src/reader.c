#include "reader.h"
#include "buckets.h"
#include "arrays.h"
#include "common.h"
#include "hash.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int index_open(index_handle_t *h, const char *buckets_path, const char *arrays_path) {
    uint64_t num_buckets = 0, hash_seed = 0;
    int bfd = buckets_open_readwrite(buckets_path, &num_buckets, &hash_seed);
    if (bfd < 0) return -1;
    int afd = arrays_open(arrays_path);
    if (afd < 0) { close(bfd); return -1; }
    h->buckets_fd = bfd;
    h->arrays_fd = afd;
    h->num_buckets = num_buckets;
    h->hash_seed = hash_seed;
    return 0;
}

void index_close(index_handle_t *h) {
    if (!h) return;
    if (h->buckets_fd >= 0) close(h->buckets_fd);
    if (h->arrays_fd >= 0) close(h->arrays_fd);
    h->buckets_fd = h->arrays_fd = -1;
}

int index_lookup(index_handle_t *h, const char *key, offset_t **out_offsets, uint32_t *out_count) {
    if (!h || !key) return -1;
    *out_offsets = NULL;
    *out_count = 0;
    uint64_t hval = hash_key_prefix20(key, strlen(key), h->hash_seed);
    uint64_t mask = h->num_buckets - 1;
    uint64_t bucket = bucket_id_from_hash(hval, mask);
    offset_t head = buckets_read_head(h->buckets_fd, h->num_buckets, bucket);
    if (head == 0) return 0;
    /* dynamic array */
    uint32_t cap = 16;
    uint32_t cnt = 0;
    offset_t *results = malloc(sizeof(offset_t) * cap);
    offset_t cur = head;
    while (cur != 0) {
        char *node_key = NULL;
        offset_t *offs = NULL;
        uint32_t list_len = 0;
        offset_t next = 0;
        if (arrays_read_node_full(h->arrays_fd, cur, &node_key, &offs, &list_len, &next) != 0) {
            /* error reading node -> break */
            break;
        }
        if (node_key) {
            /* compare exact (case-sensitive). If you want case-insensitive, modify here. */
            if (strcmp(node_key, key) == 0) {
                for (uint32_t i = 0; i < list_len; ++i) {
                    if (cnt >= cap) {
                        cap *= 2;
                        results = realloc(results, sizeof(offset_t) * cap);
                    }
                    results[cnt++] = offs[i];
                }
            }
            free(node_key);
        }
        if (offs) free(offs);
        cur = next;
    }
    if (cnt == 0) {
        free(results);
        *out_offsets = NULL;
        *out_count = 0;
    } else {
        *out_offsets = results;
        *out_count = cnt;
    }
    return 0;
}
