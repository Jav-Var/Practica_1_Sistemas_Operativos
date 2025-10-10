#ifndef BOOK_MODEL_H
#define BOOK_MODEL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct book_data {
    char *title;
    char *author_name;
    char *image_url;
    size_t num_pages;
    double average_rating;
    uint64_t text_reviews_count;
    char *description;
    
    uint64_t five_star_rating_count;
    uint64_t four_star_rating_count;
    uint64_t three_star_rating_count;
    uint64_t two_star_rating_count;
    uint64_t one_star_rating_count;
    uint64_t total_rating_count;
    
    char **genres;
    size_t genres_count;
    size_t genres_capacity;
} book_data;

#endif // BOOK_MODEL_H