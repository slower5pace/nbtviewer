#include <_types/_uint8_t.h>
#include <stdbool.h>
#include <stdio.h>
#include <zlib.h>

long decompress_gzip(const char *filename, uint8_t **out_buffer);
bool write_file_gzip(gzFile file, voidpc buf, unsigned len);
long get_file_size(FILE *f);
