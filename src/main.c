#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>   /* for access() */
#include <errno.h>

#include "util.h"
#include "builder.h"
#include "reader.h"
#include "common.h"

int main(void) {
    uint64_t hash_seed = 0x12345678abcdefULL; // 

    /* create output dir if not exists */
    struct stat st = {0};
    if (stat(INDEX_DIR, &st) == -1) {
        if (mkdir(INDEX_DIR, 0755) != 0) {
            perror("mkdir out_dir");
            return 1;
        }
    }

    /* choose num_buckets (power of two). For demo pick 4096 */
    uint64_t num_buckets_title = next_pow2(4096);
    uint64_t num_buckets_author = next_pow2(4096);

    /* prepare index file paths */
    char title_buckets[1024], title_arrays[1024];
    char author_buckets[1024], author_arrays[1024];

    snprintf(title_buckets, sizeof(title_buckets), "%s/%s_buckets.dat", INDEX_DIR, "title");
    snprintf(title_arrays, sizeof(title_arrays), "%s/%s_arrays.dat", INDEX_DIR, "title");
    snprintf(author_buckets, sizeof(author_buckets), "%s/%s_buckets.dat", INDEX_DIR, "author");
    snprintf(author_arrays, sizeof(author_arrays), "%s/%s_arrays.dat", INDEX_DIR, "author");

    /* check whether index files already present */
    int need_build = 0;
    if (access(title_buckets, F_OK) != 0) {
        printf("Missing index file: %s\n", title_buckets);
        need_build = 1;
    }
    if (access(title_arrays, F_OK) != 0) {
        printf("Missing index file: %s\n", title_arrays);
        need_build = 1;
    }
    if (access(author_buckets, F_OK) != 0) {
        printf("Missing index file: %s\n", author_buckets);
        need_build = 1;
    }
    if (access(author_arrays, F_OK) != 0) {
        printf("Missing index file: %s\n", author_arrays);
        need_build = 1;
    }

    if (need_build) {
        printf("Building indices (streaming) into '%s' ...\n", INDEX_DIR);
        if (build_both_indices_stream(CSV_PATH, INDEX_DIR, num_buckets_title, num_buckets_author, hash_seed) != 0) {
            fprintf(stderr, "Failed to build indices\n");
            return 1;
        }
        printf("Indices built.\n");
    } else {
        printf("All index files present. Skipping build.\n");
    }

    /* Open title index */
    index_handle_t ih;
    if (index_open(&ih, title_buckets, title_arrays) != 0) {
        fprintf(stderr, "Failed to open title index\n");
        return 1;
    }

    /* lookup */
    char query_title[1024];
    printf("Enter the query\n> ");
    if (fgets(query_title, sizeof(query_title), stdin) == NULL) {
        perror("fgets");
        return 1;
    }
    /* remove trailing newline if present */
    size_t len = strlen(query_title);
    if (len > 0 && query_title[len-1] == '\n') {
        query_title[len-1] = '\0';
    }

    offset_t *results = NULL;
    uint32_t count = 0;
    if (index_lookup(&ih, query_title, &results, &count) != 0) {
        fprintf(stderr, "index_lookup failed\n");
        index_close(&ih);
        return 1;
    }

    if (count == 0) {
        printf("No results for title: \"%s\"\n", query_title);
    } else {
        printf("Found %u result(s) for title: \"%s\"\n", count, query_title);
        FILE *f = fopen(CSV_PATH, "rb");
        if (!f) {
            perror("fopen combined csv");
            free(results);
            index_close(&ih);
            return 1;
        }
        for (uint32_t i = 0; i < count; ++i) {
            offset_t off = results[i];
            if (fseeko(f, (off_t)off, SEEK_SET) != 0) {
                perror("fseeko");
                continue;
            }
            char *line = NULL;
            size_t len = 0;
            ssize_t r = getline(&line, &len, f);
            if (r > 0) {
                /* print the raw CSV line (newline preserved by getline) */
                printf("---- record %u (offset %llu) ----\n", i+1, (unsigned long long)off);
                printf("%s", line);
            } else {
                printf("Failed to read line at offset %llu\n", (unsigned long long)off);
            }
            if (line) free(line);
        }
        fclose(f);
    }

    if (results) free(results);
    index_close(&ih);

    printf("Done.\n");
    return 0;
}
