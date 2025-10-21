#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>

#define BUCKETS_HEADER_SIZE 4096
#define ARRAYS_HEADER_SIZE 4096
#define BUCKET_ENTRY_SIZE 8
#define INDEX_MAGIC "IDX1" // n
#define INDEX_VERSION 1 // n

#define CSV_PATH "data/dataset/books_data.csv"
#define INDEX_DIR "data/index"
#define NUM_DATASET_FIELDS 14

typedef uint64_t offset_t;

/* safe IO wrappers */
ssize_t safe_pread(int fd, void *buf, size_t count, off_t offset);
ssize_t safe_pwrite(int fd, const void *buf, size_t count, off_t offset);

#endif // COMMON_H
