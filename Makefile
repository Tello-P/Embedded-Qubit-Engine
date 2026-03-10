CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2 -Iincludes
BIN_DIR = bin
OBJ_DIR = obj
SRC_DIR = src
INC_DIR = includes
TEST_DIR = tests

TARGET = $(BIN_DIR)/quantum_test
OBJS = $(OBJ_DIR)/fixed_point.o


all: $(TARGET)

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS) $(TEST_DIR)/test_math.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_math.c $(OBJS) -o $@

test: all
	@echo "Testing..."
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Cleaned"

.PHONY: all test clean
