#include "file.h"
#include "operations.h"
#include "parser.h"
#include "utils.h"
#include "zlib.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_null.h>

#define UNUSED(x) (void)(x)

static int print_tags = 1;

// Called even when file is not gzipped, does not matter

// Forward declarations for parsing functions

NBT_Tag *parse(uint8_t buffer[], long size) {
  long pos = 0;
  int depth = 0;
  NBT_Tag *root_compound = NULL;
  NBT_Tag *current_compound = NULL;

  while (pos < size) {
    uint8_t current = buffer[pos];

    switch (current) {
    case END:
      PRINT_TAG("%*s[END]\n", (depth - 1) * 2, "");
      if (current_compound && current_compound->value.compound_value.previous) {
        current_compound = current_compound->value.compound_value.previous;
        depth--;
      }
      pos++;
      break;

    case COMPOUND:
      parse_compound_tag(buffer, &pos, &depth, &current_compound,
                         &root_compound);
      break;

    case INT:
      parse_int_tag(buffer, &pos, depth, current_compound);
      break;

    case BYTE:
      parse_byte_tag(buffer, &pos, depth, current_compound);
      break;

    case FLOAT:
      parse_float_tag(buffer, &pos, depth, current_compound);
      break;

    case DOUBLE:
      parse_double_tag(buffer, &pos, depth, current_compound);
      break;

    case SHORT:
      parse_short_tag(buffer, &pos, depth, current_compound);
      break;

    case LONG:
      parse_long_tag(buffer, &pos, depth, current_compound);
      break;

    case STRING:
      parse_string_tag(buffer, &pos, depth, current_compound);
      break;

    case LIST:
      parse_list_tag(buffer, &pos, depth, current_compound);
      break;

    case BYTE_ARRAY:
      parse_byte_array_tag(buffer, &pos, depth, current_compound);
      break;

    case INT_ARRAY:
      parse_int_array_tag(buffer, &pos, depth, current_compound);
      break;

    case LONG_ARRAY:
      parse_long_array_tag(buffer, &pos, depth, current_compound);
      break;

    default:
      printf("Unknown tag type %d at position %ld (0x%lx)\n", current, pos,
             pos);
      printf("Context: depth=%d, current tag name=%s\n", depth,
             current_compound ? current_compound->name : "none");
      pos++;
    }
  }
  return root_compound;
}

int main(int argc, char *argv[]) {
  UNUSED(argc);
  if (argv[1] == NULL) {
    printf("Target file name not provided");
  }

  uint8_t *decompressed_data;

  long file_size = decompress_gzip(argv[1], &decompressed_data);

  NBT_Tag *root_compound = parse(decompressed_data, file_size);
  // if (argv[2] != NULL) {
  //   NBT_Tag *search_result = find_tag(root_compound, argv[2]);
  //   if (search_result == NULL) {
  //     PRINT_TAG("Could not find tag");
  //     exit(1);
  //   }
  //   PRINT_TAG("Found tag name: %s\n", search_result->name);
  // }
}
