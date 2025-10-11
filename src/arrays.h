#ifndef ARRAYS_H
#define ARRAYS_H

#include <stdint.h>
#include "common.h"

/* Functions for managing the title_arrays.dat and author_arrays.dat files
 * Each file contains a header followed by a sequence of nodes
 * Each node contains:
 * - 
 * 
*/

/* create an empty arrays file with header space reserved */
int arrays_create(const char *path);

/* open arrays file for read/write, returns the file descriptor */
int arrays_open(const char *path);

/* appends a node and return offset where node starts (absolute offset in arrays.dat) */
offset_t arrays_append_node(int fd, const char *key, const offset_t *record_offsets, uint32_t list_len, offset_t next_ptr);

/* read a node fully: caller must free key_out and offsets_out */
int arrays_read_node_full(int fd, offset_t node_off, char **key_out, offset_t **offsets_out, uint32_t *list_len_out, offset_t *next_ptr_out);


/* returns the size of a node with a key (string) of size key_len, and a list of offsets of size list_len */
size_t arrays_calc_node_size(uint16_t key_len, uint32_t list_len);

#endif // ARRAYS_H
