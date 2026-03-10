CC = gcc

CFLAGS = -Wall -Wextra -Werror -O2 -Iincludes

BIN_DIR = bin
OBJ_DIR = obj
SRC_DIR = src
TEST_DIR = tests

CORE_OBJS = $(OBJ_DIR)/fixed_point.o $(OBJ_DIR)/quantum_core.o

TARGET_MATH = $(BIN_DIR)/test_math
TARGET_QUANTUM = $(BIN_DIR)/test_quantum
TARGET_QUANTUM_EXTRA = $(BIN_DIR)/test_quantum_extra

all: $(TARGET_MATH) $(TARGET_QUANTUM) $(TARGET_QUANTUM_EXTRA)

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_MATH): $(OBJ_DIR)/fixed_point.o $(TEST_DIR)/test_math.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_math.c $(OBJ_DIR)/fixed_point.o -o $@

$(TARGET_QUANTUM): $(CORE_OBJS) $(TEST_DIR)/test_quantum.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_quantum.c $(CORE_OBJS) -o $@

$(TARGET_QUANTUM_EXTRA): $(CORE_OBJS) $(TEST_DIR)/test_quantum_extra.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_quantum_extra.c $(CORE_OBJS) -o $@

test_math: $(TARGET_MATH)
	./$(TARGET_MATH)

test_quantum: $(TARGET_QUANTUM)
	./$(TARGET_QUANTUM)

test_quantum_extra: $(TARGET_QUANTUM_EXTRA)
	./$(TARGET_QUANTUM_EXTRA)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	rm -f $(SRC_DIR)/*.o
	@echo "Cleaned"

.PHONY: all clean test_math test_quantum test_quantum_extra
