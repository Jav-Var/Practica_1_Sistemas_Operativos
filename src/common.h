#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>

#define BUCKETS_HEADER_SIZE 4096
#define ARRAYS_HEADER_SIZE 4096
#define BUCKET_ENTRY_SIZE 8
#define INDEX_MAGIC "IDX1"
#define INDEX_VERSION 1

#define CSV_PATH "data/dataset/books_data.csv"
#define INDEX_DIR "data/index"

typedef uint64_t offset_t;

/* safe IO wrappers */
ssize_t safe_pread(int fd, void *buf, size_t count, off_t offset);
ssize_t safe_pwrite(int fd, const void *buf, size_t count, off_t offset);

/* little-endian read/write helpers */
uint16_t le_read_u16(const void *buf);
uint32_t le_read_u32(const void *buf);
uint64_t le_read_u64(const void *buf);
void le_write_u16(void *buf, uint16_t v);
void le_write_u32(void *buf, uint32_t v);
void le_write_u64(void *buf, uint64_t v);

char *normalize_string(const char *s);
int normalized_strcmp(const char *a, const char *b);

#endif // COMMON_H
