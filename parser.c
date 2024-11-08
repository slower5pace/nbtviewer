#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
