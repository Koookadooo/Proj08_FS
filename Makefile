# Compiler
CC := gcc

# Compiler Flags
CFLAGS := -Wall -Wextra -Werror

# Executable Name
TARGET := testfs

# Source files and objects
SRC_DIRS := src helpers tests
SOURCES := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
OBJECTS := $(patsubst %.c, %.o, $(SOURCES))

# Library Name
LIB := libvvsfs.a

# Default rule
all: $(TARGET)

# Build the executable
$(TARGET): $(LIB) tests/testfs.o
	$(CC) tests/testfs.o -L. -lvvsfs -o $@

# Create the static library
$(LIB): $(filter-out tests/testfs.o, $(OBJECTS))
	ar rcs $@ $^

# Generic rule for compiling objects
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Test rule
test: $(TARGET)
	./$(TARGET)

# Clean rule
clean:
	rm -f $(OBJECTS) helpers/*.o tests/*.o

# Pristine rule
pristine: clean
	rm -f $(TARGET) $(LIB) *.img *.bin

# Phony targets
.PHONY: all test clean pristine
