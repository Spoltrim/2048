TARGET_EXEC = main

BUILD_DIR = build
TARGET_DIR = bin
SRC_DIR = src
H_DIR = headers

CC = gcc

# Sources
SRCS := main.c $(shell find $(SRC_DIR) -name "*.c")

# src/truc.c -> build/truc.o
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

CFLAGS = -Wall -Wextra -Werror

# Compilation de l'executable
$(TARGET_DIR)/$(TARGET_EXEC): $(OBJS)
	mkdir -p $(TARGET_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Règle de compilation pour .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET_DIR)
