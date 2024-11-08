#include "file.h"
#include "operations.h"
#include "parser.h"
#include "zlib.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_null.h>

#define UNUSED(x) (void)(x)

static int print_tags = 1;

#define PRINT_TAG(...)                                                         \
  if (print_tags) {                                                            \
    printf(__VA_ARGS__);                                                       \
  }

// Called even when file is not gzipped, does not matter

// Forward declarations for parsing functions
static void parse_compound_tag(uint8_t buffer[], long *pos, int *depth,
                               NBT_Tag **current_compound,
                               NBT_Tag **root_compound);
static void parse_int_tag(uint8_t buffer[], long *pos, int depth,
                          NBT_Tag *current_compound);
static void parse_byte_tag(uint8_t buffer[], long *pos, int depth,
                           NBT_Tag *current_compound);
static void parse_float_tag(uint8_t buffer[], long *pos, int depth,
                            NBT_Tag *current_compound);
static void parse_double_tag(uint8_t buffer[], long *pos, int depth,
                             NBT_Tag *current_compound);
static void parse_short_tag(uint8_t buffer[], long *pos, int depth,
                            NBT_Tag *current_compound);
static void parse_long_tag(uint8_t buffer[], long *pos, int depth,
                           NBT_Tag *current_compound);
static void parse_string_tag(uint8_t buffer[], long *pos, int depth,
                             NBT_Tag *current_compound);
static void parse_list_tag(uint8_t buffer[], long *pos, int depth,
                           NBT_Tag *current_compound);
static void parse_byte_array_tag(uint8_t buffer[], long *pos, int depth,
                                 NBT_Tag *current_compound);
static void parse_int_array_tag(uint8_t buffer[], long *pos, int depth,
                                NBT_Tag *current_compound);
static void parse_long_array_tag(uint8_t buffer[], long *pos, int depth,
                                 NBT_Tag *current_compound);

static void parse_compound_tag(uint8_t buffer[], long *pos, int *depth,
                               NBT_Tag **current_compound,
                               NBT_Tag **root_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  PRINT_TAG("%*s[COMPOUND] %s\n", *depth * 2, "", name);

  if (*root_compound == NULL) {
    *root_compound = create_compound(NULL, name, name_len);
    *current_compound = *root_compound;
  } else {
    NBT_Tag *new_compound = create_compound(*current_compound, name, name_len);
    add_tag_to_compound(*current_compound, new_compound);
    *current_compound =
        &(*current_compound)
             ->value.compound_value
             .elements[(*current_compound)->value.compound_value.length - 1];
  }
  (*depth)++;
}

static void parse_int_tag(uint8_t buffer[], long *pos, int depth,
                          NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  int32_t value = get_int(buffer, pos);
  PRINT_TAG("%*s[INT] %s = %d\n", depth * 2, "", name, value);
  NBT_Tag *tag = create_int_tag(name, name_len, value);
  add_tag_to_compound(current_compound, tag);
}

static void parse_byte_tag(uint8_t buffer[], long *pos, int depth,
                           NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  int8_t value = get_byte(buffer, pos);
  PRINT_TAG("%*s[BYTE] %s = %hhx\n", depth * 2, "", name, value);
  NBT_Tag *tag = create_byte_tag(name, name_len, value);
  add_tag_to_compound(current_compound, tag);
}

static void parse_float_tag(uint8_t buffer[], long *pos, int depth,
                            NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  float value = get_float(buffer, pos);
  PRINT_TAG("%*s[FLOAT] %s = %.2f\n", depth * 2, "", name, value);
  NBT_Tag *tag = create_float_tag(name, name_len, value);
  add_tag_to_compound(current_compound, tag);
}

static void parse_double_tag(uint8_t buffer[], long *pos, int depth,
                             NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  double_t value = get_double(buffer, pos);
  PRINT_TAG("%*s[DOUBLE] %s = %.4f\n", depth * 2, "", name, value);
  NBT_Tag *tag = create_double_tag(name, name_len, value);
  add_tag_to_compound(current_compound, tag);
}

static void parse_short_tag(uint8_t buffer[], long *pos, int depth,
                            NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  int16_t value = get_short(buffer, pos);
  PRINT_TAG("%*s[SHORT] %s = %hu\n", depth * 2, "", name, value);
  NBT_Tag *tag = create_short_tag(name, name_len, value);
  add_tag_to_compound(current_compound, tag);
}

static void parse_long_tag(uint8_t buffer[], long *pos, int depth,
                           NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  int64_t value = get_long(buffer, pos);
  PRINT_TAG("%*s[LONG] %s = %lld\n", depth * 2, "", name, (long long)value);
  NBT_Tag *tag = create_long_tag(name, name_len, value);
  add_tag_to_compound(current_compound, tag);
}

static void parse_string_tag(uint8_t buffer[], long *pos, int depth,
                             NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  uint16_t str_len = get_len_short(buffer, pos);
  char *string_content = get_text_short(buffer, pos, str_len);
  PRINT_TAG("%*s[STRING] %s = %s\n", depth * 2, "", name, string_content);

  NBT_Tag *str = create_string_tag(name, name_len, string_content, str_len);
  add_tag_to_compound(current_compound, str);
  free(string_content);
}

static void parse_list_tag(uint8_t buffer[], long *pos, int depth,
                           NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  enum TagType element_type = buffer[*pos];
  (*pos)++;
  int32_t list_size = get_int(buffer, pos);
  NBT_Tag *elements = malloc(sizeof(NBT_Tag) * list_size);

  if (!elements) {
    printf("Failed to allocate memory for list elements\n");
    free(name);
    exit(1);
  }

  PRINT_TAG("%*s[LIST] %s: length=%d\n", depth * 2, "", name, list_size);

  for (int32_t i = 0; i < list_size; i++) {
    switch (element_type) {
    case BYTE: {
      int8_t value = get_byte(buffer, pos);
      elements[i] = *create_byte_tag(NULL, 0, value);
      PRINT_TAG("%*s  [%d] = %d\n", depth * 2, "", i, value);
      break;
    }
    case SHORT: {
      int16_t value = get_short(buffer, pos);
      elements[i] = *create_short_tag(NULL, 0, value);
      PRINT_TAG("%*s  [%d] = %d\n", depth * 2, "", i, value);
      break;
    }
    case INT: {
      int32_t value = get_int(buffer, pos);
      elements[i] = *create_int_tag(NULL, 0, value);
      PRINT_TAG("%*s  [%d] = %d\n", depth * 2, "", i, value);
      break;
    }
    case LONG: {
      int64_t value = get_long(buffer, pos);
      elements[i] = *create_long_tag(NULL, 0, value);
      PRINT_TAG("%*s  [%d] = %lld\n", depth * 2, "", i, (long long)value);
      break;
    }
    case FLOAT: {
      float value = get_float(buffer, pos);
      elements[i] = *create_float_tag(NULL, 0, value);
      PRINT_TAG("%*s  [%d] = %.2f\n", depth * 2, "", i, value);
      break;
    }
    case DOUBLE: {
      double value = get_double(buffer, pos);
      elements[i] = *create_double_tag(NULL, 0, value);
      PRINT_TAG("%*s  [%d] = %.4f\n", depth * 2, "", i, value);
      break;
    }
    case STRING: {
      uint16_t str_len = get_len_short(buffer, pos);
      char *string_content = get_text_short(buffer, pos, str_len);
      elements[i] = *create_string_tag(NULL, 0, string_content, str_len);
      PRINT_TAG("%*s  [%d] = %s\n", depth * 2, "", i, string_content);
      free(string_content);
      break;
    }
    case BYTE_ARRAY: {
      int32_t array_len = get_int(buffer, pos);
      int8_t *data = malloc(array_len * sizeof(int8_t));
      if (!data) {
        printf("Failed to allocate memory for byte array in list\n");
        free(elements);
        free(name);
        exit(1);
      }
      for (int32_t j = 0; j < array_len; j++) {
        data[j] = get_byte(buffer, pos);
      }
      elements[i] = *create_byte_array_tag(NULL, 0, data, array_len);
      PRINT_TAG("%*s  [%d] = byte[%d]\n", depth * 2, "", i, array_len);
      free(data);
      break;
    }
    case INT_ARRAY: {
      int32_t array_len = get_int(buffer, pos);
      int32_t *data = malloc(array_len * sizeof(int32_t));
      if (!data) {
        printf("Failed to allocate memory for int array in list\n");
        free(elements);
        free(name);
        exit(1);
      }
      for (int32_t j = 0; j < array_len; j++) {
        data[j] = get_int(buffer, pos);
      }
      elements[i] = *create_int_array_tag(NULL, 0, data, array_len);
      PRINT_TAG("%*s  [%d] = int[%d]\n", depth * 2, "", i, array_len);
      free(data);
      break;
    }
    case LONG_ARRAY: {
      int32_t array_len = get_int(buffer, pos);
      int64_t *data = malloc(array_len * sizeof(int64_t));
      if (!data) {
        printf("Failed to allocate memory for long array in list\n");
        free(elements);
        free(name);
        exit(1);
      }
      for (int32_t j = 0; j < array_len; j++) {
        data[j] = get_long(buffer, pos);
      }
      elements[i] = *create_long_array_tag(NULL, 0, data, array_len);
      PRINT_TAG("%*s  [%d] = long[%d]\n", depth * 2, "", i, array_len);
      free(data);
      break;
    }
    case COMPOUND: {
      // Handling compounds inside list
      NBT_Tag *compound_tag = create_compound(NULL, NULL, 0);
      if (!compound_tag) {
        printf("Failed to create compound tag in list\n");
        free(elements);
        free(name);
        exit(1);
      }

      int nested_depth = depth + 1;
      PRINT_TAG("%*s  [%d] = compound\n", depth * 2, "", i);

      // Parse nested compound
      while (buffer[*pos] != END) {
        uint8_t tag_type = buffer[*pos];
        switch (tag_type) {
        case BYTE:
          parse_byte_tag(buffer, pos, nested_depth + 1, compound_tag);
          break;
        case SHORT:
          parse_short_tag(buffer, pos, nested_depth + 1, compound_tag);
          break;
        case INT:
          parse_int_tag(buffer, pos, nested_depth + 1, compound_tag);
          break;
        case LONG:
          parse_long_tag(buffer, pos, nested_depth + 1, compound_tag);
          break;
        case FLOAT:
          parse_float_tag(buffer, pos, nested_depth + 1, compound_tag);
          break;
        case DOUBLE:
          parse_double_tag(buffer, pos, nested_depth + 1, compound_tag);
          break;
        case STRING:
          parse_string_tag(buffer, pos, nested_depth + 1, compound_tag);
          break;
        case BYTE_ARRAY:
          parse_byte_array_tag(buffer, pos, nested_depth + 1, compound_tag);
          break;
        case INT_ARRAY:
          parse_int_array_tag(buffer, pos, nested_depth + 1, compound_tag);
          break;
        case LONG_ARRAY:
          parse_long_array_tag(buffer, pos, nested_depth + 1, compound_tag);
          break;
        default:
          printf("Unexpected tag type %d in compound list element\n", tag_type);
          free(elements);
          free(name);
          free_tag(compound_tag);
          exit(1);
        }
      }
      // Move past the END tag
      (*pos)++;
      elements[i] = *compound_tag;
      free(compound_tag);
      break;
    }
    case LIST: {
      enum TagType nested_element_type = buffer[*pos];
      (*pos)++;
      int32_t nested_list_size = get_int(buffer, pos);
      NBT_Tag *nested_elements = malloc(sizeof(NBT_Tag) * nested_list_size);
      if (!nested_elements) {
        printf("Failed to allocate memory for nested list\n");
        free(elements);
        free(name);
        exit(1);
      }

      PRINT_TAG("%*s  [%d] = list[%d]\n", depth * 2, "", i, nested_list_size);

      NBT_Tag *nested_list = create_list_tag(NULL, 0, nested_element_type,
                                             nested_list_size, nested_elements);
      if (!nested_list) {
        printf("Failed to create nested list tag\n");
        free(nested_elements);
        free(elements);
        free(name);
        exit(1);
      }

      elements[i] = *nested_list;
      free(nested_list);
      break;
    }
    default:
      printf("Unsupported list element type: %d\n", element_type);
      free(elements);
      free(name);
      exit(1);
    }
  }

  NBT_Tag *list_tag =
      create_list_tag(name, name_len, element_type, list_size, elements);
  if (!list_tag) {
    printf("Failed to create list tag\n");
    free(elements);
    free(name);
    exit(1);
  }

  add_tag_to_compound(current_compound, list_tag);
}

static void parse_byte_array_tag(uint8_t buffer[], long *pos, int depth,
                                 NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  int32_t length = get_int(buffer, pos);
  int8_t *data = malloc(length * sizeof(int8_t));

  for (int32_t i = 0; i < length; i++) {
    data[i] = get_byte(buffer, pos);
  }

  PRINT_TAG("%*s[BYTE_ARRAY] %s: length=%d\n", depth * 2, "", name, length);
  NBT_Tag *array_tag = create_byte_array_tag(name, name_len, data, length);
  add_tag_to_compound(current_compound, array_tag);
  free(data);
}

static void parse_int_array_tag(uint8_t buffer[], long *pos, int depth,
                                NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  int32_t length = get_int(buffer, pos);
  int32_t *data = malloc(length * sizeof(int32_t));

  for (int32_t i = 0; i < length; i++) {
    data[i] = get_int(buffer, pos);
  }

  PRINT_TAG("%*s[INT_ARRAY] %s: length=%d\n", depth * 2, "", name, length);
  NBT_Tag *array_tag = create_int_array_tag(name, name_len, data, length);
  add_tag_to_compound(current_compound, array_tag);
  free(data);
}

static void parse_long_array_tag(uint8_t buffer[], long *pos, int depth,
                                 NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  int32_t length = get_int(buffer, pos);
  int64_t *data = malloc(length * sizeof(int64_t));

  for (int32_t i = 0; i < length; i++) {
    data[i] = get_long(buffer, pos);
  }

  PRINT_TAG("%*s[LONG_ARRAY] %s: length=%d\n", depth * 2, "", name, length);
  NBT_Tag *array_tag = create_long_array_tag(name, name_len, data, length);
  add_tag_to_compound(current_compound, array_tag);
  free(data);
}

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
