# Tests

`make test` compila y ejecuta los tests C (cualquier fallo → `exit(1)`) y después `pytest`
dos veces: contra el motor de escritorio (`src/*.c`) y contra la copia C++ del firmware
(`QuantumCore/src/*.cpp`, compilada con `g++` en el host). Requiere `gcc`, `g++`, `python3`,
`numpy` y `pytest`.

Si un test no puede fallar, no prueba nada. Por eso: (1) todas las comprobaciones comparan contra
una referencia independiente (NumPy) o contra una propiedad física (norma 1), (2) los tests de
medida usan estados donde el resultado no es trivial, y (3) se comprobó con mutaciones (abajo) que
los tests se rompen cuando el motor se rompe.

## Qué se prueba

| Archivo | Qué y por qué |
|---|---|
| `tests/qsim_run.c` | Runner: `qsim_run N [--seed S] h 0 cx 0 1 m 0`. Estado crudo (`real imag`, Q1.14) por stdout, medidas (`M q res`) por stderr. Puertas: `h x z cx m`, además de `o K` (`apply_oracle`) y `d` (`apply_diffuser`) para Grover. |
| `tests/reference.py` | Simulador NumPy `complex128`. Qubit 0 = bit menos significativo (igual que `mask = 1 << q` del C; lo verifican `test_qubit_order` y `test_reference_matches_c_order`). |
| `tests/test_engine.py` | 79 tests pytest: escala Q1.14 (H\|0⟩ ≈ 11585), orden de qubits, CNOT, norma = 1 (vacío, Bell, GHZ-6, H en 4 y 6 qubits), amplitudes frente a NumPy, 50 circuitos aleatorios (semilla fija), colapso de Bell (20 semillas), medida de estados ya definidos y de un estado no estabilizador (Grover 3q), GHZ-6 medido, estadística 50/50, Grover 2 qubits (P ≥ 0.99, 4 objetivos) y 3–5 qubits (P = valor teórico). |
| `tests/test_*.c` | Los tests C originales, que solo imprimían, ahora usan `CHECK` y `assert_norm()` (`tests/test_common.h`) tras cada circuito: Bell + colapso, teleportación (100 semillas, comprobando que salen los 4 síndromes), Grover 3q (8 objetivos), Grover 2q, registros, GHZ hasta `MAX_QUBITS`, puertas de 1 qubit, `fp_mul`/`complex_mag_sq` frente a `double`. |

Por qué los estados "no triviales": con H, X, Z y CNOT todo estado es estabilizador y cada medida
tiene probabilidad 0, ½ ó 1. Un `measure_qubit` que renormalizase como si todo fuera ½ pasaría
Bell y GHZ. Grover de 3 qubits (P(marcado) = 0.945) saca el estado de ese conjunto.

## Tolerancias

Medido con `python3 tests/measure_error.py` (500 circuitos aleatorios de 1–6 qubits por profundidad,
más los circuitos conocidos). 1 LSB = 1/16384 = 6.1e-5.

| Magnitud | Máx. medido | Tolerancia | Justificación |
|---|---|---|---|
| Error por amplitud, circuitos conocidos (≤ 6 puertas) | 0.24 LSB | 2 LSB | Cada H redondea ±0.5 LSB; con ≤ 6 H el error acumulado no puede pasar de ~3 y en la práctica no llega a 1. |
| Error por amplitud, aleatorios | 1.2 (prof. 5), 2 (10, 20), **4.24 (40)**, 6 (80) LSB | 8 LSB | Crece como un paseo aleatorio con la profundidad; 8 ≈ 2× el máximo medido a prof. 40 (el máximo del test). |
| \|norma − 1\| | 3.7e-4 (prof. 40), 6.7e-4 (prof. 80) | 2e-3 | ≈ 3× el máximo medido; un error de bulto (como el bug 1: norma 1.41) la supera de sobra. |
| Fidelidad frente a NumPy | 1 − F ≤ 1.2e-7 | > 0.99 | Umbral pedido; es muy laxo (F = 0.99 permite un error de ~10 % en amplitud), por eso los tests también comparan amplitudes. |
| Grover n ≥ 3 | — | ± 0.03 sobre sin²((2k+1)·asin(1/√N)) | Error de redondeo en la probabilidad ≪ 0.03; un diffuser u oráculo mal hecho se aparta más de 0.1. |
| Estadística 50/50 | — | ± 5σ (p. ej. 1000 ± 112 de 2000) | Con semilla fija es determinista; 5σ evita un test frágil si cambia el RNG. |

**Error máximo medido frente a NumPy: 4.24 LSB (2.6e-4 en amplitud) en circuitos de hasta 40
puertas; 6 LSB con 80 puertas.**

Límite conocido: las tolerancias no distinguen redondeo de truncado en `apply_gate_h` (error de ~1 LSB
por puerta, dentro de la tolerancia). Es una implementación válida a esta precisión.

## Comprobación por mutación

Se inyectó cada fallo en una copia de `src/` y se ejecutó pytest. Todos se detectan salvo el último:

| Mutación | Resultado |
|---|---|
| H con 11000 en vez de 1/√2 | 19 tests fallan |
| CNOT con control/target invertidos | 17 |
| Z sin cambio de signo en la parte real | 2 |
| Medida con el resultado invertido | 4 |
| Oráculo marca el índice `k ^ 1` | 31 |
| X con el orden de qubits invertido | 10 |
| Diffuser sin inversión de fase | 31 |
| Estado inicial \|1…⟩ → 0 | 79 |
| H que trunca en vez de redondear | **no detectada** (dentro de tolerancia) |

## Bugs encontrados

### 1. `measure_qubit` renormaliza con un divisor fijo (corregido)

- **Pasos:** `build/qsim_run 1 m 0` (medir un qubit en \|0⟩). También `qsim_run 3 h 0 h 1 h 2 o 5 d o 5 d m 0` (Grover 3q, cualquier semilla).
- **Esperado:** el estado conserva norma 1 (\|0⟩ → amplitud 16384).
- **Obtenido:** amplitud 23170, norma 1.414. En el estado de Grover, norma 1.39 y amplitudes que no coinciden con la proyección renormalizada. El código dividía siempre por 11585 (= 1/√2), válido solo si la probabilidad del resultado era ½; por eso Bell y la teleportación no lo mostraban.
- **Seed:** independiente de la semilla (el resultado es casi determinista en esos estados).
- **Detectado por:** `test_measure_deterministic_keeps_norm`, `test_measure_nonuniform_state_matches_numpy`, `test_measure_two_qubits_sequence_ghz` (pytest) y `test_quantum.c`.
- **Corrección** (solo `src/quantum_core.c` y su copia `QuantumCore/src/quantum_core.cpp`, separable con `git diff -- src QuantumCore/src`): se acumula Σ(re²+im²) en Q28 sin truncar, se calcula su raíz entera (`isqrt_u32`, sin FPU ni división, apta para AVR) y se divide por ella.

## Cambios no funcionales necesarios para poder probar

- `includes/quantum_register.h`: `MAX_QUBITS` pasa a `#ifndef` (el valor por defecto sigue en 4). El Makefile compila con `-DMAX_QUBITS=6`, igual que el firmware, para poder probar GHZ de 6 qubits.
- `test_quantum_extra.c` y `various.c` usan la API antigua (`apply_gate_h(qubit_t*)`) y no compilaban; quedan fuera de `all`/`test`. Su cobertura (X, H, normalización, estadística, colapso) está en `test_quantum.c` y pytest.

## Lo que no se prueba

- El ensamblador AVR de `fp_mul` (`#ifdef __AVR__`): el CI solo comprueba que el firmware **compila** con `avr-g++`; no se ejecuta (haría falta simavr o hardware). Los tests de escritorio usan la rama C de `fp_mul`.
- `QuantumCore.ino` (necesita el core de Arduino).
