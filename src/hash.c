/* hash.c  -- modificar la función hash_key_prefix20 para usar normalize_string
   (reemplaza la implementación anterior). */
#include "hash.h"
#include "util.h" /* para normalize_string */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Keep same HASH_KEY_PREFIX_LEN (20) from hash.h */

/* FNV-1a 64-bit over up to first N normalized characters, mixed with seed. */
uint64_t hash_key_prefix20(const char *key, size_t len, uint64_t seed) {
    const uint64_t FNV_OFFSET = 14695981039346656037ULL;
    const uint64_t FNV_PRIME = 1099511628211ULL;

    /* Normalize key (malloc'd). If normalize fails, fallback to raw key bytes. */
    char *norm = normalize_string(key ? key : "");
    const unsigned char *data = NULL;
    size_t max = 0;

    if (norm) {
        data = (const unsigned char *)norm;
        size_t nlen = strlen(norm);
        max = (nlen < HASH_KEY_PREFIX_LEN) ? nlen : HASH_KEY_PREFIX_LEN;
    } else {
        data = (const unsigned char *)key;
        max = (len < HASH_KEY_PREFIX_LEN) ? len : HASH_KEY_PREFIX_LEN;
    }

    uint64_t h = FNV_OFFSET ^ seed;
    for (size_t i = 0; i < max; ++i) {
        h ^= (uint64_t)data[i];
        h *= FNV_PRIME;
    }

    /* avalanche mixing (same as before) */
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;

    if (norm) free(norm);
    return h;
}
