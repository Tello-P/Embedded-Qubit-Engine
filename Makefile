CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2 -Iincludes

BIN_DIR = bin
OBJ_DIR = obj
SRC_DIR = src
TEST_DIR = tests

FIXED_POINT_O      = $(OBJ_DIR)/fixed_point.o
QUANTUM_CORE_O     = $(OBJ_DIR)/quantum_core.o
QUANTUM_REGISTER_O = $(OBJ_DIR)/quantum_register.o

CORE_OBJS = $(FIXED_POINT_O) $(QUANTUM_CORE_O)
FULL_OBJS = $(FIXED_POINT_O) $(QUANTUM_CORE_O) $(QUANTUM_REGISTER_O)

TARGET_MATH              = $(BIN_DIR)/test_math
TARGET_QUANTUM           = $(BIN_DIR)/test_quantum
TARGET_QUANTUM_EXTRA     = $(BIN_DIR)/test_quantum_extra
TARGET_QUANTUM_REGISTER  = $(BIN_DIR)/test_quantum_register
TARGET_MULTIPLE_QUBITS   = $(BIN_DIR)/test_multiple_qubits
TARGET_VARIOUS           = $(BIN_DIR)/various
TARGET_GROVER = $(BIN_DIR)/test_grover
TARGET_BELL_STATE = $(BIN_DIR)/test_bell_state
TARGET_TELEPORT = $(BIN_DIR)/test_teleport
TARGET_GROVER_VISUAL = $(BIN_DIR)/test_grover_visual


all: $(TARGET_MATH) $(TARGET_QUANTUM) $(TARGET_QUANTUM_EXTRA) $(TARGET_QUANTUM_REGISTER) $(TARGET_MULTIPLE_QUBITS) $(TARGET_VARIOUS) $(TARGET_GROVER) $(TARGET_BELL_STATE) $(TARGET_TELEPORT) $(TARGET_GROVER_VISUAL)


$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_MATH): $(FIXED_POINT_O) $(TEST_DIR)/test_math.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_math.c $(FIXED_POINT_O) -o $@

$(TARGET_QUANTUM): $(CORE_OBJS) $(TEST_DIR)/test_quantum.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_quantum.c $(CORE_OBJS) -o $@

$(TARGET_QUANTUM_EXTRA): $(CORE_OBJS) $(TEST_DIR)/test_quantum_extra.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_quantum_extra.c $(CORE_OBJS) -o $@

$(TARGET_QUANTUM_REGISTER): $(FULL_OBJS) $(TEST_DIR)/test_quantum_register.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_quantum_register.c $(FULL_OBJS) -o $@

$(TARGET_MULTIPLE_QUBITS): $(FULL_OBJS) $(TEST_DIR)/test_multiple_qubits.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_multiple_qubits.c $(FULL_OBJS) -o $@

$(TARGET_VARIOUS): $(FULL_OBJS) $(TEST_DIR)/various.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/various.c $(FULL_OBJS) -o $@

$(TARGET_GROVER): $(FULL_OBJS) $(TEST_DIR)/test_grover.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_grover.c $(FULL_OBJS) -o $@

$(TARGET_BELL_STATE): $(FULL_OBJS) $(TEST_DIR)/test_bell_state.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_bell_state.c $(FULL_OBJS) -o $@

$(TARGET_TELEPORT): $(FULL_OBJS) $(TEST_DIR)/test_teleport.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_teleport.c $(FULL_OBJS) -o $@

$(TARGET_GROVER_VISUAL): $(FULL_OBJS) $(TEST_DIR)/test_grover_visual.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_grover_visual.c $(FULL_OBJS) -o $@


test_math: $(TARGET_MATH)
	./$(TARGET_MATH)

test_quantum: $(TARGET_QUANTUM)
	./$(TARGET_QUANTUM)

test_quantum_extra: $(TARGET_QUANTUM_EXTRA)
	./$(TARGET_QUANTUM_EXTRA)

test_quantum_register: $(TARGET_QUANTUM_REGISTER)
	./$(TARGET_QUANTUM_REGISTER)

test_multiple_qubits: $(TARGET_MULTIPLE_QUBITS)
	./$(TARGET_MULTIPLE_QUBITS)

test_various: $(TARGET_VARIOUS)
	./$(TARGET_VARIOUS)

test_grover: $(TARGET_GROVER)
	./$(TARGET_GROVER) $(n)

test_bell: $(TARGET_BELL_STATE)
	./$(TARGET_BELL_STATE)

test_teleport: $(TARGET_TELEPORT)
	./$(TARGET_TELEPORT)

test_grover_visual: $(TARGET_GROVER_VISUAL)
	./$(TARGET_GROVER_VISUAL)


clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Cleaned"

.PHONY: all clean test_math test_quantum test_quantum_extra test_quantum_register test_multiple_qubits test_various test_grover_visual
