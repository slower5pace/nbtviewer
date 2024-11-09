#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int print_tags = 1;

#define PRINT_TAG(...)                                                         \
  if (print_tags) {                                                            \
    printf(__VA_ARGS__);                                                       \
  }

inline uint16_t get_len_short(uint8_t *buf, long *pos) {
  // little endian to big endian
  uint16_t val = (buf[*pos] << 8) | buf[*pos + 1];
  // if (val == 0) {
  //   printf("Tag name length is 0 at position %ld", *pos);
  //   exit(EXIT_FAILURE);
  // }
  *pos += 2;
  return val;
}

// for getting tag name/text content
inline char *get_text_short(uint8_t *buf, long *pos, uint16_t len) {
  //  if (len == 0) {
  //    return NULL;
  //  }
  //
  char *name = malloc(len + 1);
  if (name == NULL) {
    return NULL;
  }

  memcpy(name, &buf[*pos], len);
  name[len] = '\0';

  *pos += len;
  return name;
}

NBT_Tag *create_compound(NBT_Tag *previous, char *name, uint16_t name_len) {
  NBT_Tag *tag = malloc(sizeof(NBT_Tag));
  if (tag == NULL) {
    printf("Could not allocate memory for tag %s", name);
  }
  tag->name = name;
  tag->tag_type = COMPOUND;
  tag->name_length = name_len;
  tag->value.compound_value.previous = previous;
  // length and capacity is dynamic since we cannot know
  // how big the compound is until we reach the END tag
  tag->value.compound_value.length = 0;
  tag->value.compound_value.capacity = 8;
  tag->value.compound_value.elements = malloc(sizeof(NBT_Tag) * 8);
  return tag;
}

void add_tag_to_compound(NBT_Tag *compound, NBT_Tag *child) {
  if (compound == NULL) {
    printf("Cannot add tag to null compound");
    exit(1);
  }

  if (compound->tag_type != COMPOUND) {
    printf("The tag %s is not a compound tag, cannot add children",
           compound->name);
  }

  if (compound->value.compound_value.capacity ==
      compound->value.compound_value.length) {
    NBT_Tag *new_elements =
        realloc(compound->value.compound_value.elements,
                sizeof(NBT_Tag) * compound->value.compound_value.capacity * 2);

    if (new_elements == NULL) {
      printf("Failed to resize compound tag elements array\n");
      exit(1);
    }

    compound->value.compound_value.elements = new_elements;
    compound->value.compound_value.capacity =
        compound->value.compound_value.capacity * 2;
  }

  compound->value.compound_value
      .elements[compound->value.compound_value.length++] = *child;

  free(child);
}

// Helper function for common tag initialization
inline NBT_Tag *init_tag(enum TagType type, char *name, uint16_t name_len) {
  NBT_Tag *tag = malloc(sizeof(NBT_Tag));
  if (tag == NULL) {
    printf("Could not allocate memory for tag %s\n", name);
    return NULL;
  }

  tag->tag_type = type;
  tag->name = name;
  tag->name_length = name_len;
  return tag;
}

void free_tag(NBT_Tag *tag) {
  if (tag == NULL) {
    return;
  }

  free(tag->name);

  switch (tag->tag_type) {
  case STRING:
    free(tag->value.string_value.data);
    break;

  case LIST:
    // for (int i = 0; i < tag->value.list_value.length; i++) {
    //   free_tag(&tag->value.list_value.elements[i]);
    // }
    free(tag->value.list_value.elements);
    break;

  case COMPOUND:
    for (int i = 0; i < tag->value.compound_value.length; i++) {
      free_tag(&tag->value.compound_value.elements[i]);
    }
    free(tag->value.compound_value.elements);
    break;

  case BYTE_ARRAY:
    free(tag->value.byte_array.data);
    break;

  case INT_ARRAY:
    free(tag->value.int_array.data);
    break;

  case LONG_ARRAY:
    free(tag->value.long_array.data);
    break;

  // no additional memory to be freed here
  case END:
  case BYTE:
  case SHORT:
  case INT:
  case LONG:
  case FLOAT:
  case DOUBLE:
    break;

  default:
    printf("Warning: Unknown tag type %d in free_tag\n", tag->tag_type);
    break;
  }
  free(tag);
}

NBT_Tag *create_byte_tag(char *name, uint16_t name_len, int8_t value) {
  NBT_Tag *tag = init_tag(BYTE, name, name_len);
  if (tag) {
    tag->value.byte_value = value;
  }
  return tag;
}

NBT_Tag *create_short_tag(char *name, uint16_t name_len, int16_t value) {
  NBT_Tag *tag = init_tag(SHORT, name, name_len);
  if (tag) {
    tag->value.short_value = value;
  }
  return tag;
}

NBT_Tag *create_int_tag(char *name, uint16_t name_len, int32_t value) {
  NBT_Tag *tag = init_tag(INT, name, name_len);
  if (tag) {
    tag->value.int_value = value;
  }
  return tag;
}

NBT_Tag *create_long_tag(char *name, uint16_t name_len, int64_t value) {
  NBT_Tag *tag = init_tag(LONG, name, name_len);
  if (tag) {
    tag->value.long_value = value;
  }
  return tag;
}

NBT_Tag *create_float_tag(char *name, uint16_t name_len, float value) {
  NBT_Tag *tag = init_tag(FLOAT, name, name_len);
  if (tag) {
    tag->value.float_value = value;
  }
  return tag;
}

NBT_Tag *create_double_tag(char *name, uint16_t name_len, double value) {
  NBT_Tag *tag = init_tag(DOUBLE, name, name_len);
  if (tag) {
    tag->value.double_value = value;
  }
  return tag;
}

// String tag
NBT_Tag *create_string_tag(char *name, uint16_t name_len, const char *value,
                           uint16_t value_len) {
  NBT_Tag *tag = init_tag(STRING, name, name_len);
  if (!tag)
    return NULL;

  tag->value.string_value.data = malloc(value_len + 1);
  if (!tag->value.string_value.data) {
    printf("Could not allocate memory for string value\n");
    free_tag(tag);
    return NULL;
  }

  memcpy(tag->value.string_value.data, value, value_len);
  tag->value.string_value.data[value_len] = '\0';
  tag->value.string_value.length = value_len;
  return tag;
}

// Array tags
NBT_Tag *create_byte_array_tag(char *name, uint16_t name_len, int8_t *data,
                               int32_t length) {
  NBT_Tag *tag = init_tag(BYTE_ARRAY, name, name_len);
  if (!tag)
    return NULL;

  tag->value.byte_array.data = malloc(length * sizeof(int8_t));
  if (!tag->value.byte_array.data) {
    printf("Could not allocate memory for byte array\n");
    free_tag(tag);
    return NULL;
  }

  memcpy(tag->value.byte_array.data, data, length * sizeof(int8_t));
  tag->value.byte_array.length = length;
  return tag;
}

NBT_Tag *create_int_array_tag(char *name, uint16_t name_len, int32_t *data,
                              int32_t length) {
  NBT_Tag *tag = init_tag(INT_ARRAY, name, name_len);
  if (!tag)
    return NULL;

  tag->value.int_array.length = length;
  tag->value.int_array.data = data;
  return tag;
}

NBT_Tag *create_long_array_tag(char *name, uint16_t name_len, int64_t *data,
                               int32_t length) {
  NBT_Tag *tag = init_tag(LONG_ARRAY, name, name_len);
  if (!tag)
    return NULL;

  tag->value.long_array.length = length;
  tag->value.long_array.data = data;
  return tag;
}

NBT_Tag *create_list_tag(char *name, uint16_t name_len,
                         enum TagType element_type, int32_t list_len,
                         NBT_Tag *tags) {
  NBT_Tag *tag = init_tag(LIST, name, name_len);
  if (!tag)
    return NULL;

  tag->value.list_value.length = list_len;
  tag->value.list_value.element_type = element_type;
  tag->value.list_value.elements = tags;
  return tag;
}

// shifts change the endianness

inline int32_t get_int(uint8_t *buf, long *pos) {
  int32_t value = (buf[*pos] << 24) | (buf[*pos + 1] << 16) |
                  (buf[*pos + 2] << 8) | buf[*pos + 3];
  *pos += 4;
  return value;
}

inline int16_t get_short(uint8_t *buf, long *pos) {
  int16_t value = (buf[*pos + 1] << 8) | buf[*pos];
  *pos += 2;
  return value;
}

inline float get_float(uint8_t *buf, long *pos) {
  uint32_t value = ((uint32_t)buf[*pos] << 24) |
                   ((uint32_t)buf[*pos + 1] << 16) |
                   ((uint32_t)buf[*pos + 2] << 8) | (uint32_t)buf[*pos + 3];
  float result;
  memcpy(&result, &value, 4);
  *pos += 4;
  return result;
}

inline int8_t get_byte(uint8_t *buf, long *pos) {
  int8_t value = buf[*pos];
  *pos += 1;
  return value;
}

inline int64_t get_long(uint8_t *buf, long *pos) {
  int64_t value =
      ((int64_t)buf[*pos] << 56) | ((int64_t)buf[*pos + 1] << 48) |
      ((int64_t)buf[*pos + 2] << 40) | ((int64_t)buf[*pos + 3] << 32) |
      ((int64_t)buf[*pos + 4] << 24) | ((int64_t)buf[*pos + 5] << 16) |
      ((int64_t)buf[*pos + 6] << 8) | (int64_t)buf[*pos + 7];
  *pos += 8;
  return value;
}

inline double get_double(uint8_t *buf, long *pos) {
  uint64_t value =
      ((uint64_t)buf[*pos] << 56) | ((uint64_t)buf[*pos + 1] << 48) |
      ((uint64_t)buf[*pos + 2] << 40) | ((uint64_t)buf[*pos + 3] << 32) |
      ((uint64_t)buf[*pos + 4] << 24) | ((uint64_t)buf[*pos + 5] << 16) |
      ((uint64_t)buf[*pos + 6] << 8) | (uint64_t)buf[*pos + 7];
  double result;
  memcpy(&result, &value, 8);
  *pos += 8;
  return result;
}

void parse_compound_tag(uint8_t buffer[], long *pos, int *depth,
                        NBT_Tag **current_compound, NBT_Tag **root_compound) {
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

void parse_int_tag(uint8_t buffer[], long *pos, int depth,
                   NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  int32_t value = get_int(buffer, pos);
  PRINT_TAG("%*s[INT] %s = %d\n", depth * 2, "", name, value);
  NBT_Tag *tag = create_int_tag(name, name_len, value);
  add_tag_to_compound(current_compound, tag);
}

void parse_byte_tag(uint8_t buffer[], long *pos, int depth,
                    NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  int8_t value = get_byte(buffer, pos);
  PRINT_TAG("%*s[BYTE] %s = %hhx\n", depth * 2, "", name, value);
  NBT_Tag *tag = create_byte_tag(name, name_len, value);
  add_tag_to_compound(current_compound, tag);
}

void parse_float_tag(uint8_t buffer[], long *pos, int depth,
                     NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  float value = get_float(buffer, pos);
  PRINT_TAG("%*s[FLOAT] %s = %.2f\n", depth * 2, "", name, value);
  NBT_Tag *tag = create_float_tag(name, name_len, value);
  add_tag_to_compound(current_compound, tag);
}

void parse_double_tag(uint8_t buffer[], long *pos, int depth,
                      NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  double value = get_double(buffer, pos);
  PRINT_TAG("%*s[DOUBLE] %s = %.4f\n", depth * 2, "", name, value);
  NBT_Tag *tag = create_double_tag(name, name_len, value);
  add_tag_to_compound(current_compound, tag);
}

void parse_short_tag(uint8_t buffer[], long *pos, int depth,
                     NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  int16_t value = get_short(buffer, pos);
  PRINT_TAG("%*s[SHORT] %s = %hu\n", depth * 2, "", name, value);
  NBT_Tag *tag = create_short_tag(name, name_len, value);
  add_tag_to_compound(current_compound, tag);
}

void parse_long_tag(uint8_t buffer[], long *pos, int depth,
                    NBT_Tag *current_compound) {
  (*pos)++;
  uint16_t name_len = get_len_short(buffer, pos);
  char *name = get_text_short(buffer, pos, name_len);
  int64_t value = get_long(buffer, pos);
  PRINT_TAG("%*s[LONG] %s = %lld\n", depth * 2, "", name, (long long)value);
  NBT_Tag *tag = create_long_tag(name, name_len, value);
  add_tag_to_compound(current_compound, tag);
}

void parse_string_tag(uint8_t buffer[], long *pos, int depth,
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

void parse_list_tag(uint8_t buffer[], long *pos, int depth,
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

void parse_byte_array_tag(uint8_t buffer[], long *pos, int depth,
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

void parse_int_array_tag(uint8_t buffer[], long *pos, int depth,
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

void parse_long_array_tag(uint8_t buffer[], long *pos, int depth,
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
