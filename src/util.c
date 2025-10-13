#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

uint64_t next_pow2(uint64_t v) {
    if (v == 0) return 1;
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    v++;
    return v;
}

char *trim_inplace(char *s) {
    if (!s) return s;
    char *end;
    while (isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

char *xstrdup(const char *s) {
    if (!s) return NULL;
    char *d = malloc(strlen(s) + 1);
    if (!d) return NULL;
    strcpy(d, s);
    return d;
}


// EN: src/util.c
void normalize_string_to_buffer(const char *in, char *out, size_t out_size) {
    if (!in || !out || out_size == 0) {
        if (out && out_size > 0) out[0] = '\0';
        return;
    }

    size_t i = 0;
    size_t out_idx = 0;
    
    while (in[i] != '\0' && out_idx < out_size - 1) {
        unsigned char c1 = in[i];

        // Manejo de caracteres ASCII de 1 byte (los más comunes)
        if (c1 < 128) {
            out[out_idx++] = tolower(c1);
            i++;
            continue;
        }

        // Manejo de caracteres UTF-8 de 2 bytes para español
        if (c1 == 0xc3) {
            unsigned char c2 = in[i+1];
            switch (c2) {
                case 0x81: // Á
                case 0xa1: // á
                    out[out_idx++] = 'a';
                    break;
                case 0x89: // É
                case 0xa9: // é
                    out[out_idx++] = 'e';
                    break;
                case 0x8d: // Í
                case 0xad: // í
                    out[out_idx++] = 'i';
                    break;
                case 0x93: // Ó
                case 0xb3: // ó
                    out[out_idx++] = 'o';
                    break;
                case 0x9a: // Ú
                case 0xba: // ú
                    out[out_idx++] = 'u';
                    break;
                case 0x91: // Ñ
                case 0xb1: // ñ -> CORREGIDO
                    out[out_idx++] = 'n';
                    break;
                default:
                    // Si es un carácter UTF-8 que no manejamos, lo ignoramos
                    break;
            }
            i += 2; // Avanzamos 2 bytes
        } else {
            // Si es otro carácter multibyte que no reconocemos, lo ignoramos
            i++;
        }
    }
    
    out[out_idx] = '\0';
}