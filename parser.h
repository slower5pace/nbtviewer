#ifndef NBT_TAGS_H
#define NBT_TAGS_H

#include <stdint.h>

// Tag type enumeration
enum TagType {
  END,
  BYTE,
  SHORT,
  INT,
  LONG,
  FLOAT,
  DOUBLE,
  BYTE_ARRAY,
  STRING,
  LIST,
  COMPOUND,
  INT_ARRAY,
  LONG_ARRAY
};

// Value union to store different tag types
union NBT_Value {
  int8_t byte_value;
  int8_t bool_value;
  int16_t short_value;
  int32_t int_value;
  int64_t long_value;
  float float_value;
  double double_value;
  struct {
    char *data;
    uint16_t length;
  } string_value;
  struct {
    int8_t *data;
    int32_t length;
  } byte_array;
  struct {
    int32_t *data;
    int32_t length;
  } int_array;
  struct {
    int64_t *data;
    int32_t length;
  } long_array;
  struct {
    struct NBT_Tag *elements;
    int32_t length;
    enum TagType element_type;
  } list_value;
  struct {
    struct NBT_Tag *previous;
    struct NBT_Tag *elements;
    // Compounds do not normally have length nor capacity fields, these
    // are for parsing purposes only.
    int32_t length;
    int32_t capacity;
  } compound_value;
};

// NBT Tag structure
typedef struct NBT_Tag {
  enum TagType tag_type;
  uint16_t name_length;
  char *name;
  union NBT_Value value;
} NBT_Tag;

void parse_compound_tag(uint8_t buffer[], long *pos, int *depth,
                        NBT_Tag **current_compound, NBT_Tag **root_compound);
void parse_int_tag(uint8_t buffer[], long *pos, int depth,
                   NBT_Tag *current_compound);
void parse_byte_tag(uint8_t buffer[], long *pos, int depth,
                    NBT_Tag *current_compound);
void parse_float_tag(uint8_t buffer[], long *pos, int depth,
                     NBT_Tag *current_compound);
void parse_double_tag(uint8_t buffer[], long *pos, int depth,
                      NBT_Tag *current_compound);
void parse_short_tag(uint8_t buffer[], long *pos, int depth,
                     NBT_Tag *current_compound);
void parse_long_tag(uint8_t buffer[], long *pos, int depth,
                    NBT_Tag *current_compound);
void parse_string_tag(uint8_t buffer[], long *pos, int depth,
                      NBT_Tag *current_compound);
void parse_list_tag(uint8_t buffer[], long *pos, int depth,
                    NBT_Tag *current_compound);
void parse_byte_array_tag(uint8_t buffer[], long *pos, int depth,
                          NBT_Tag *current_compound);
void parse_int_array_tag(uint8_t buffer[], long *pos, int depth,
                         NBT_Tag *current_compound);
void parse_long_array_tag(uint8_t buffer[], long *pos, int depth,
                          NBT_Tag *current_compound);

// Tag creation functions
NBT_Tag *create_compound(NBT_Tag *previous, char *name, uint16_t name_len);
NBT_Tag *create_byte_tag(char *name, uint16_t name_len, int8_t value);
NBT_Tag *create_short_tag(char *name, uint16_t name_len, int16_t value);
NBT_Tag *create_int_tag(char *name, uint16_t name_len, int32_t value);
NBT_Tag *create_long_tag(char *name, uint16_t name_len, int64_t value);
NBT_Tag *create_float_tag(char *name, uint16_t name_len, float value);
NBT_Tag *create_double_tag(char *name, uint16_t name_len, double value);
NBT_Tag *create_string_tag(char *name, uint16_t name_len, const char *value,
                           uint16_t value_len);
NBT_Tag *create_byte_array_tag(char *name, uint16_t name_len, int8_t *data,
                               int32_t length);
NBT_Tag *create_int_array_tag(char *name, uint16_t name_len, int32_t *data,
                              int32_t length);
NBT_Tag *create_long_array_tag(char *name, uint16_t name_len, int64_t *data,
                               int32_t length);
NBT_Tag *create_list_tag(char *name, uint16_t name_len,
                         enum TagType element_type, int32_t list_len,
                         NBT_Tag *tags);

NBT_Tag *init_tag(enum TagType type, char *name, uint16_t name_len);
void free_tag(NBT_Tag *tag);
void add_tag_to_compound(NBT_Tag *compound, NBT_Tag *child);

// Helper / utility functions
uint16_t get_len_short(uint8_t *buf, long *pos);
char *get_text_short(uint8_t *buf, long *pos, uint16_t len);
int32_t get_int(uint8_t *buf, long *pos);
int16_t get_short(uint8_t *buf, long *pos);
float get_float(uint8_t *buf, long *pos);
int8_t get_byte(uint8_t *buf, long *pos);
int64_t get_long(uint8_t *buf, long *pos);
double get_double(uint8_t *buf, long *pos);

#endif // NBT_TAGS_H
