#ifndef BUILDER_H
#define BUILDER_H

#include <stdint.h>

/* Build a single index (index_name: "title" or "author") from combined CSV.
   csv_path: path to combined_dataset.csv
   out_dir: directory where to write index files; files will be:
     out_dir/index_name_buckets.dat
     out_dir/index_name_arrays.dat
   num_buckets: number of buckets (must be power of two)
   hash_seed: seed for hash_key prefix
*/

int build_index_stream(const char *csv_path, const char *out_dir, const char *index_name, uint64_t num_buckets, uint64_t hash_seed);

/* Build both indices title and author */
int build_both_indices_stream(const char *csv_path, const char *out_dir, uint64_t num_buckets_title, uint64_t num_buckets_author, uint64_t hash_seed);

#endif // BUILDER_H
