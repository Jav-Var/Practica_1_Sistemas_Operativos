#include "hash.h"

/* Simple FNV-1a 64-bit over first up to 20 bytes, mixed with seed.
   FNV-1a is simple and fast. We mix seed at the beginning.
*/
uint64_t hash_key_prefix20(const char *key, size_t len, uint64_t seed) {
    const uint64_t FNV_OFFSET = 14695981039346656037ULL;
    const uint64_t FNV_PRIME = 1099511628211ULL;
    uint64_t h = FNV_OFFSET ^ seed;
    size_t max = len < HASH_KEY_PREFIX_LEN ? len : HASH_KEY_PREFIX_LEN;
    for (size_t i = 0; i < max; ++i) {
        unsigned char c = (unsigned char)key[i];
        h ^= (uint64_t)c;
        h *= FNV_PRIME;
    }
    /* further avalanche */
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    return h;
}
