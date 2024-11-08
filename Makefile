CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lz
SOURCES = main.c parser.c operations.c file.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = nbt_viewer

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -rf $(TARGET).dSYM

.PHONY: all clean
