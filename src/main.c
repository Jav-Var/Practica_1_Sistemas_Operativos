
#define  _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>

#include "util.h"
#include "reader.h"

int main(void) {
    const char *csv_path = "data/dataset/books_data.csv";
    const char *out_dir = "data/index";
    const char *query_title = "The Little Prince Macmillan Collectors Library";

    /* Paths to prebuilt index files */
    char title_buckets[1024], title_arrays[1024];
    snprintf(title_buckets, sizeof(title_buckets), "%s/%s_buckets.dat", out_dir, "title");
    snprintf(title_arrays, sizeof(title_arrays), "%s/%s_arrays.dat", out_dir, "title");

    /* Quick existence check */
    struct stat st;
    if (stat(title_buckets, &st) != 0) {
        fprintf(stderr, "Buckets file not found: %s\n", title_buckets);
        return 1;
    }
    if (stat(title_arrays, &st) != 0) {
        fprintf(stderr, "Arrays file not found: %s\n", title_arrays);
        return 1;
    }
    if (stat(csv_path, &st) != 0) {
        fprintf(stderr, "CSV file not found: %s\n", csv_path);
        return 1;
    }

    /* Open title index */
    index_handle_t ih;
    if (index_open(&ih, title_buckets, title_arrays) != 0) {
        fprintf(stderr, "Failed to open title index\n");
        return 1;
    }

    /* lookup */
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
        FILE *f = fopen(csv_path, "rb");
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
