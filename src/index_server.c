// index_server.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include "reader.h"    // index_handle_t, index_open, index_close
// #include "builder.h" // si necesitas construir índices desde aquí
// #include "your_index_api.h" // replace with your actual headers

#define REQ_FIFO "/tmp/index_req.fifo"
#define RSP_FIFO "/tmp/index_rsp.fifo"
#define BUF_SZ 8192

/* Helper: ensure FIFO exists */
static int ensure_fifo(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        if (!S_ISFIFO(st.st_mode)) {
            fprintf(stderr, "Path exists and is not FIFO: %s\n", path);
            return -1;
        }
        return 0;
    }
    if (mkfifo(path, 0666) != 0) {
        perror("mkfifo");
        return -1;
    }
    return 0;
}

/* Read one line (up to newline) from fd into malloc'd buffer (caller frees) */
static char *read_line_fd(int fd) {
    char *buf = malloc(BUF_SZ);
    if (!buf) return NULL;
    size_t pos = 0;
    while (1) {
        ssize_t r = read(fd, buf + pos, 1);
        if (r <= 0) {
            if (pos == 0) { free(buf); return NULL; } // EOF or error
            break;
        }
        if (buf[pos] == '\n') { buf[pos] = '\0'; return buf; }
        pos++;
        if (pos + 1 >= BUF_SZ) break; // too long; truncate
    }
    buf[pos] = '\0';
    return buf;
}

/* Write line to fd, adding newline */
static int write_line_fd(int fd, const char *s) {
    size_t len = strlen(s);
    ssize_t w = write(fd, s, len);
    if (w != (ssize_t)len) return -1;
    if (write(fd, "\n", 1) != 1) return -1;
    return 0;
}

/* MAIN */
int main() {
    const char *index_dir = INDEX_DIR;
    const char *csv_path = CSV_PATH;

    if (ensure_fifo(REQ_FIFO) != 0) return 1;
    if (ensure_fifo(RSP_FIFO) != 0) return 1;

    /* open FIFOs O_RDWR to avoid blocking on open if no peer yet */
    int req_fd = open(REQ_FIFO, O_RDWR);
    if (req_fd < 0) { perror("open req fifo"); return 1; }
    int rsp_fd = open(RSP_FIFO, O_RDWR);
    if (rsp_fd < 0) { perror("open rsp fifo"); close(req_fd); return 1; }

    /* Open indices once (at server start). Adjust paths to your files. */
    char title_buckets[1024], title_arrays[1024], author_buckets[1024], author_arrays[1024];
    snprintf(title_buckets, sizeof(title_buckets), "%s/title_buckets.dat", index_dir);
    snprintf(title_arrays, sizeof(title_arrays), "%s/title_arrays.dat", index_dir);
    snprintf(author_buckets, sizeof(author_buckets), "%s/author_buckets.dat", index_dir);
    snprintf(author_arrays, sizeof(author_arrays), "%s/author_arrays.dat", index_dir);

    index_handle_t th, ah;
    if (index_open(&th, title_buckets, title_arrays) != 0) {
        fprintf(stderr, "Failed to open title index\n");
        // but keep running or exit?
    }
    if (index_open(&ah, author_buckets, author_arrays) != 0) {
        fprintf(stderr, "Failed to open author index\n");
    }

    printf("Server listening. Send queries as 'TITLE|AUTHOR' lines to %s\n", REQ_FIFO);

    while (1) {
        char *req = read_line_fd(req_fd);
        if (!req) {
            // sleep a bit on EOF/error and continue
            usleep(100000);
            continue;
        }
        // parse request: split on first '|'
        char *sep = strchr(req, '|');
        char *title = NULL;
        char *author = NULL;
        if (sep) {
            *sep = '\0';
            title = req;
            author = sep + 1;
        } else {
            title = req;
            author = "";
        }

        if ((title[0] == '\0') && (author[0] == '\0')) {
            write_line_fd(rsp_fd, "ERR|Search must have at least one parameter");
            write_line_fd(rsp_fd, "<END>");
            free(req);
            continue;
        }

        /* Now perform lookup: we use lookup_by_title_author which returns offsets.
           Adapt this to your API: it expects open index handles and returns offsets. */
        printf("Searching title: %s, author: %s\n", title, author);
        offset_t *offs = NULL;
        uint32_t count = 0;
        int rc = lookup_by_title_author(&th, &ah, title, author, &offs, &count);
        if (rc != 0) {
            write_line_fd(rsp_fd, "ERR|Internal lookup error");
            write_line_fd(rsp_fd, "<END>");
            free(req);
            continue;
        }

        if (count == 0) {
            write_line_fd(rsp_fd, "OK");
            write_line_fd(rsp_fd, "<END>");
            free(req);
            continue;
        }

        /* For each offset read the CSV line and write it to response FIFO */
        FILE *csvf = fopen(csv_path, "rb");
        if (!csvf) {
            write_line_fd(rsp_fd, "ERR|Cannot open CSV file");
            write_line_fd(rsp_fd, "<END>");
            free(offs);
            free(req);
            continue;
        }

        write_line_fd(rsp_fd, "OK");
        for (uint32_t i = 0; i < count; ++i) {
            off_t off = (off_t)offs[i];
            if (fseeko(csvf, off, SEEK_SET) != 0) {
                // skip
                continue;
            }
            char *line = NULL;
            size_t llen = 0;
            ssize_t r = getline(&line, &llen, csvf);
            if (r > 0) {
                // strip trailing newline for neat output (optional)
                if (line[r-1] == '\n') line[r-1] = '\0';
                write_line_fd(rsp_fd, line);
            }
            free(line);
        }
        write_line_fd(rsp_fd, "<END>");

        free(offs);
        fclose(csvf);
        free(req);
    }

    /* cleanup (never reached in this loop) */
    index_close(&th);
    index_close(&ah);
    close(req_fd);
    close(rsp_fd);
    return 0;
}
