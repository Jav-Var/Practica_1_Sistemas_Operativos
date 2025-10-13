/* src/ui_client.c
 *
 * Interfaz simple en español que comunica con el index_server mediante FIFOs:
 *  - request FIFO: /tmp/index_req.fifo
 *  - response FIFO: /tmp/index_rsp.fifo
 *
 * Menú:
 *   Bienvenido.
 *   1. Ingresar titulo
 *   2. Ingresar autor
 *   3. Realizar Busqueda
 *   4. Salir
 *
 * Nota: este cliente es single-client. Para múltiples clientes simultáneos
 * convendría que cada cliente cree su FIFO de respuesta propio y lo pase al servidor.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define REQ_FIFO "/tmp/index_req.fifo"
#define RSP_FIFO "/tmp/index_rsp.fifo"
#define MAX_LINE 8192

/* Trim trailing newline/carriage return */
static void rtrim_newline(char *s) {
    if (!s) return;
    size_t L = strlen(s);
    while (L > 0 && (s[L-1] == '\n' || s[L-1] == '\r')) {
        s[--L] = '\0';
    }
}

/* Return printable representation for current value */
static const char *display_or_empty(const char *s) {
    return (s != NULL && s[0] != '\0') ? s : "(vacío)";
}

/* write a line (without trailing newline) to fd and append '\n' */
static int write_line_fd(int fd, const char *s) {
    size_t len = strlen(s);
    ssize_t w = write(fd, s, len);
    if (w != (ssize_t)len) return -1;
    if (write(fd, "\n", 1) != 1) return -1;
    return 0;
}

/* Read response from rsp_fd and print until "<END>" is found.
   Uses fdopen on a dup so it doesn't close original fd used elsewhere. */
static void read_and_print_response(int rsp_fd) {
    FILE *f = fdopen(dup(rsp_fd), "r");
    if (!f) {
        perror("fdopen respuesta");
        return;
    }
    char buf[MAX_LINE];
    while (fgets(buf, sizeof(buf), f)) {
        rtrim_newline(buf);
        if (strcmp(buf, "<END>") == 0) break;
        if (strncmp(buf, "ERR|", 4) == 0) {
            printf("ERROR (server): %s\n", buf + 4);
        } else if (strcmp(buf, "OK") == 0) {
            /* header OK: skip printing */
        } else {
            /* CSV line or normal response line */
            printf("%s\n", buf);
        }
    }
    fclose(f);
}

/* Read a trimmed line from stdin (malloc'd). Caller must free.
   Returns NULL on EOF or error. */
static char *getline_trimmed_stdin(void) {
    char *line = NULL;
    size_t len = 0;
    ssize_t r = getline(&line, &len, stdin);
    if (r <= 0) {
        if (line) free(line);
        return NULL;
    }
    rtrim_newline(line);
    /* trim leading spaces */
    char *s = line;
    while (*s && (*s == ' ' || *s == '\t')) s++;
    if (s != line) memmove(line, s, strlen(s) + 1);
    /* trim trailing spaces */
    size_t L = strlen(line);
    while (L > 0 && (line[L-1] == ' ' || line[L-1] == '\t')) line[--L] = '\0';
    return line;
}

int main(void) {
    printf("Interfaz de búsqueda - cliente\n");

    /* Check FIFOs existence */
    if (access(REQ_FIFO, F_OK) != 0 || access(RSP_FIFO, F_OK) != 0) {
        fprintf(stderr, "Aviso: no se encuentran las FIFOs (%s, %s).\n"
                        "Asegúrate de que index_server esté corriendo y haya creado las FIFOs.\n",
                REQ_FIFO, RSP_FIFO);
        /* continue: attempt to open may block or fail */
    }

    int req_fd = open(REQ_FIFO, O_RDWR);
    if (req_fd < 0) {
        fprintf(stderr, "No pude abrir FIFO de peticiones '%s': %s\n", REQ_FIFO, strerror(errno));
        return 1;
    }
    int rsp_fd = open(RSP_FIFO, O_RDWR);
    if (rsp_fd < 0) {
        fprintf(stderr, "No pude abrir FIFO de respuestas '%s': %s\n", RSP_FIFO, strerror(errno));
        close(req_fd);
        return 1;
    }

    char *current_title = NULL;
    char *current_author = NULL;

    while (1) {
        printf("\nBienvenido.\n");
        printf("Título actual: %s\n", display_or_empty(current_title));
        printf("Autor actual : %s\n", display_or_empty(current_author));
        printf("\n1. Ingresar titulo\n");
        printf("2. Ingresar autor\n");
        printf("3. Realizar Busqueda\n");
        printf("4. Salir\n");
        printf("Selecciona una opción: ");
        fflush(stdout);

        char *opt = getline_trimmed_stdin();
        if (!opt) {
            printf("\nEOF o error de entrada. Saliendo.\n");
            break;
        }

        if (strcmp(opt, "1") == 0) {
            printf("Ingrese titulo (enter para dejar vacío): ");
            char *t = getline_trimmed_stdin();
            printf("Se ingreso %s\n", t);
            if (t && t[0] == '\0') { free(t); t = NULL; }
            free(current_title);
            current_title = t;
        } else if (strcmp(opt, "2") == 0) {
            printf("Ingrese autor (enter para dejar vacío): ");
            char *a = getline_trimmed_stdin();
            printf("Se ingreso %s\n", a);
            if (a && a[0] == '\0') { free(a); a = NULL; }
            free(current_author);
            current_author = a;
        } else if (strcmp(opt, "3") == 0) {
            const char *t = current_title ? current_title : "";
            const char *a = current_author ? current_author : "";
            if (t[0] == '\0' && a[0] == '\0') {
                printf("Error: la búsqueda debe tener al menos un parámetro (titulo o autor).\n");
                free(opt);
                continue;
            }

            /* Compose request safely (use empty strings if NULL) */
            char req[MAX_LINE];
            snprintf(req, sizeof(req), "%s|%s", t, a);

            if (write_line_fd(req_fd, req) != 0) {
                fprintf(stderr, "Error escribiendo petición en FIFO: %s\n", strerror(errno));
            } else {
                /* Read response and print */
                read_and_print_response(rsp_fd);
            }
        } else if (strcmp(opt, "4") == 0) {
            free(opt);
            break;
        } else {
            printf("Opción no válida. Por favor elige 1..4.\n");
        }

        free(opt);
    }

    free(current_title);
    free(current_author);
    close(req_fd);
    close(rsp_fd);
    printf("Cliente finalizado.\n");
    return 0;
}
