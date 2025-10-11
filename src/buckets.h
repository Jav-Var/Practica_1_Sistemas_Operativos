#ifndef BUCKETS_H
#define BUCKETS_H

#include <stdint.h>
#include "common.h"

/* buckets.h
 * Functions for managing the title_buckets.dat and author_buckets.dat files
 * Each buckets file contains a header followed by a contiguous table of bucket entries.
 *
 * File layout:
 * - header (fixed size BUCKETS_HEADER_SIZE bytes)
 *   - magic         : 4 bytes  (ASCII, e.g. "IDX1")
 *   - version       : uint16  (2 bytes)
 *   - reserved      : uint16  (2 bytes)
 *   - page_size     : uint32  (4 bytes)  // typically BUCKETS_HEADER_SIZE
 *   - num_buckets   : uint64  (8 bytes)  // number of bucket entries (prefer power-of-two)
 *   - hash_seed     : uint64  (8 bytes)  // seed used by the hash function
 *   - entry_size    : uint32  (4 bytes)  // size of each bucket entry in bytes (typically 8)
 *   - reserved/pad  : rest of header to fill BUCKETS_HEADER_SIZE
 *
 * - bucket table (num_buckets entries, fixed-size entries)
 *   For i = 0 .. num_buckets-1:
 *     bucket[i].head_offset : uint64 (8 bytes)
 *
 * Semantics:
 * - Each bucket entry stores an **absolute byte offset** into the corresponding arrays.dat file,
 *   pointing to the first node of the chain for that bucket.
 * - head_offset == 0 means the bucket is empty (no nodes).
 * - Offsets are absolute from the start of arrays.dat (i.e., the value suitable to pass to pread).
 *
 * Notes and invariants:
 * - BUCKETS_HEADER_SIZE bytes at the beginning of the file are reserved for header metadata.
 *   Bucket entries start at offset BUCKETS_HEADER_SIZE.
 * - Using fixed-size entries allows O(1) seek to any bucket: entry_offset = BUCKETS_HEADER_SIZE + bucket_id * entry_size.
 * - It is common to use num_buckets as a power-of-two and compute bucket_id = hash & (num_buckets - 1).
 * - Writing a bucket head is usually a single atomic write of 8 bytes (platform-dependent). For crash-safety
 *   consider ordering writes (append node in arrays.dat first, then update bucket head) and/or using WAL or fsync.
 * - For concurrency, protect updates to a single bucket (locks or atomic CAS) to avoid lost updates.
 *
 * This module exposes functions to create/open the buckets file, read a bucket head, and write a bucket head.
 */

/* Create buckets file with header and num_buckets entries zeroed */
int buckets_create(const char *path, uint64_t num_buckets, uint64_t hash_seed);

/* Open buckets file and read header (returns fd or -1) */
int buckets_open_readwrite(const char *path, uint64_t *num_buckets_out, uint64_t *hash_seed_out);

/* Read head offset for bucket_id (0..num_buckets-1) */
offset_t buckets_read_head(int fd, uint64_t num_buckets, uint64_t bucket_id);

/* Write head offset for bucket_id */
int buckets_write_head(int fd, uint64_t num_buckets, uint64_t bucket_id, offset_t head);

/* Helper to compute offset in file for bucket entry */
off_t buckets_entry_offset(uint64_t bucket_id);

#endif // BUCKETS_H
