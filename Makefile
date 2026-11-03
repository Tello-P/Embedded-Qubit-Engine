CC = gcc
# El firmware AVR usa 6 qubits; el escritorio se compila igual para poder probarlos.
MAX_QUBITS ?= 6
CFLAGS = -Wall -Wextra -Werror -O2 -Iincludes -DMAX_QUBITS=$(MAX_QUBITS)

BIN_DIR = bin
BUILD_DIR = build
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


# test_quantum_extra.c y various.c usan la API antigua (puertas sobre qubit_t*) y no
# compilan: se dejan fuera de `all`/`test` hasta que se porten al registro.
C_TESTS = $(TARGET_MATH) $(TARGET_QUANTUM) $(TARGET_QUANTUM_REGISTER) $(TARGET_MULTIPLE_QUBITS) \
          $(TARGET_GROVER) $(TARGET_BELL_STATE) $(TARGET_TELEPORT) $(TARGET_GROVER_VISUAL)

QSIM_RUN = $(BUILD_DIR)/qsim_run
QSIM_RUN_FW = $(BUILD_DIR)/qsim_run_fw

all: $(C_TESTS) $(QSIM_RUN)


$(BIN_DIR) $(OBJ_DIR) $(BUILD_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(wildcard includes/*.h) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_MATH): $(FIXED_POINT_O) $(TEST_DIR)/test_math.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_math.c $(FIXED_POINT_O) -o $@

$(TARGET_QUANTUM): $(FULL_OBJS) $(TEST_DIR)/test_quantum.c $(TEST_DIR)/test_common.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_quantum.c $(FULL_OBJS) -o $@ -lm

$(TARGET_QUANTUM_EXTRA): $(CORE_OBJS) $(TEST_DIR)/test_quantum_extra.c $(TEST_DIR)/test_common.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_quantum_extra.c $(CORE_OBJS) -o $@

$(TARGET_QUANTUM_REGISTER): $(FULL_OBJS) $(TEST_DIR)/test_quantum_register.c $(TEST_DIR)/test_common.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_quantum_register.c $(FULL_OBJS) -o $@ -lm

$(TARGET_MULTIPLE_QUBITS): $(FULL_OBJS) $(TEST_DIR)/test_multiple_qubits.c $(TEST_DIR)/test_common.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_multiple_qubits.c $(FULL_OBJS) -o $@ -lm

$(TARGET_VARIOUS): $(FULL_OBJS) $(TEST_DIR)/various.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/various.c $(FULL_OBJS) -o $@

$(TARGET_GROVER): $(FULL_OBJS) $(TEST_DIR)/test_grover.c $(TEST_DIR)/test_common.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_grover.c $(FULL_OBJS) -o $@ -lm

$(TARGET_BELL_STATE): $(FULL_OBJS) $(TEST_DIR)/test_bell_state.c $(TEST_DIR)/test_common.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_bell_state.c $(FULL_OBJS) -o $@ -lm

$(TARGET_TELEPORT): $(FULL_OBJS) $(TEST_DIR)/test_teleport.c $(TEST_DIR)/test_common.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_teleport.c $(FULL_OBJS) -o $@ -lm

$(TARGET_GROVER_VISUAL): $(FULL_OBJS) $(TEST_DIR)/test_grover_visual.c $(TEST_DIR)/test_common.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_grover_visual.c $(FULL_OBJS) -o $@ -lm


$(QSIM_RUN): $(FULL_OBJS) $(TEST_DIR)/qsim_run.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/qsim_run.c $(FULL_OBJS) -o $@ -lm

# Mismo runner compilado con las fuentes C++ del firmware (QuantumCore/src), en el host.
$(QSIM_RUN_FW): $(wildcard QuantumCore/src/*) $(TEST_DIR)/qsim_run.c | $(BUILD_DIR)
	g++ -O2 -w -IQuantumCore/src -x c++ $(TEST_DIR)/qsim_run.c QuantumCore/src/*.cpp -o $@

# make test: ejecuta los tests C (fallan con exit code != 0) y despues pytest.
test: $(C_TESTS) $(QSIM_RUN) $(QSIM_RUN_FW)
	@set -e; for t in $(C_TESTS); do echo "== $$t"; ./$$t; done
	python3 -m pytest $(TEST_DIR) -q
	QSIM_BIN=$(QSIM_RUN_FW) python3 -m pytest $(TEST_DIR) -q

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
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(BUILD_DIR) .pytest_cache $(TEST_DIR)/__pycache__
	@echo "Cleaned"

.PHONY: all test clean test_math test_quantum test_quantum_extra test_quantum_register test_multiple_qubits test_various test_grover_visual
