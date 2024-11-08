
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

long decompress_gzip(const char *filename, uint8_t **out_buffer) {
  gzFile gz = gzopen(filename, "rb");
  if (gz == NULL) {
    printf("Could not open gzip file: %s\n", filename);
    return -1;
  }

  // Arbitrarily choosen buffer size
  size_t buffer_size = 20 * 1024;
  size_t total_size = 0;
  uint8_t *buffer = malloc(buffer_size);

  if (buffer == NULL) {
    printf("Initial memory allocation for file buffer failed\n");
    gzclose(gz);
    return -1;
  }

  while (1) {
    if (total_size == buffer_size) {
      buffer_size *= 2;
      uint8_t *new_buffer = realloc(buffer, buffer_size);
      if (new_buffer == NULL) {
        printf("Memory reallocation for file buffer failed\n");
        free(buffer);
        gzclose(gz);
        return -1;
      }
      buffer = new_buffer;
    }

    int bytes_read = gzread(gz, buffer + total_size, buffer_size - total_size);

    if (bytes_read < 0) {
      int err;
      const char *error_string = gzerror(gz, &err);
      printf("Error reading gzip file: %s\n", error_string);
      free(buffer);
      gzclose(gz);
      return -1;
    }

    if (bytes_read == 0) {
      // End of file
      break;
    }

    total_size += bytes_read;
  }

  // Check for errors during decompression
  int err;
  const char *error_string = gzerror(gz, &err);
  if (err != Z_OK && err != Z_STREAM_END) {
    printf("Decompression error: %s\n", error_string);
    free(buffer);
    gzclose(gz);
    return -1;
  }

  gzclose(gz);

  // Shrink buffer to actual size
  if (total_size < buffer_size) {
    uint8_t *final_buffer = realloc(buffer, total_size);
    if (final_buffer != NULL) {
      buffer = final_buffer;
    }
    // If realloc fails, we'll just use the larger buffer
  }

  *out_buffer = buffer;
  return total_size;
}

long get_file_size(FILE *f) {
  if (f == NULL) {
    printf("Cannot get file size - file not found!\n");
    return 1;
  }
  fseek(f, 0L, SEEK_END);

  long res = ftell(f);
  fseek(f, 0L, SEEK_SET);
  return res;
}

// int write_file_gzip(const char *filename) {
//   gzFile gz = gzopen(filename, "wb");
//   if (gz == NULL) {
//     printf("Could not open file: %s\n", filename);
//     return -1;
//   }
// }
