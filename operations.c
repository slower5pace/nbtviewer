#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

NBT_Tag *find_tag(NBT_Tag *compound, const char *name) {
  if (compound->tag_type != COMPOUND) {
    printf("Tag is not a compound tag");
    exit(1);
  }
  for (int i = 0; i < compound->value.compound_value.length; i++) {
    if (strcmp(compound->value.compound_value.elements[i].name, name) == 0) {
      return &compound->value.compound_value.elements[i];
    }

    if (compound->value.compound_value.elements[i].tag_type == COMPOUND) {
      NBT_Tag *tag =
          find_tag(&compound->value.compound_value.elements[i], name);
      if (tag != NULL) {
        return tag;
      }
    }
  }
  return NULL;
}

// TODO: someday
void edit_tag(NBT_Tag *tag, union NBT_Value value) { tag->value = value; }
