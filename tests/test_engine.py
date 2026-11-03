"""Tests del motor cuantico C contra un simulador NumPy de referencia."""
import numpy as np
import pytest

import reference as ref
from engine import run_c, ONE

# Tolerancias (justificacion y mediciones en docs/tests.md)
KNOWN_TOL = 2 / ONE    # circuitos cortos (<=6 puertas): medido 0.24 LSB
AMP_TOL = 8 / ONE      # circuitos aleatorios de hasta 40 puertas: medido 4.24 LSB
NORM_TOL = 2e-3        # |norma - 1|: medido <= 3.7e-4 (prof 40), 6.7e-4 (prof 80)
FID_MIN = 0.99         # pedido por la especificacion; medido 1-F <= 6e-8


def random_circuit(rng, n, depth):
    gates = []
    for _ in range(depth):
        k = rng.choice(["h", "x", "z", "cx"] if n > 1 else ["h", "x", "z"])
        if k == "cx":
            c, t = rng.choice(n, size=2, replace=False)
            gates.append(("cx", int(c), int(t)))
        else:
            gates.append((str(k), int(rng.integers(n))))
    return gates


KNOWN = {
    "vacio": (3, []),
    "bell": (2, [("h", 0), ("cx", 0, 1)]),
    "ghz6": (6, [("h", 0)] + [("cx", 0, q) for q in range(1, 6)]),
    "h_todos_6": (6, [("h", q) for q in range(6)]),
    "h_todos_4": (4, [("h", q) for q in range(4)]),
}


def probs(c):
    return np.abs(c) ** 2


# ---------------------------------------------------------------- escala/orden
def test_q14_scale_hadamard():
    raw, _, _ = run_c(1, ["h", 0])
    assert abs(raw[0, 0] - 11585) <= 1 and abs(raw[1, 0] - 11585) <= 1
    assert raw[0, 1] == 0 and raw[1, 1] == 0


def test_initial_state_is_one():
    raw, _, _ = run_c(3, [])
    assert raw[0].tolist() == [16384, 0]
    assert not raw[1:].any()


@pytest.mark.parametrize("q", range(6))
def test_qubit_order(q):
    """X en el qubit q -> indice 1<<q (qubit 0 = bit menos significativo)."""
    raw, _, _ = run_c(6, ["x", q])
    assert raw[1 << q].tolist() == [16384, 0]
    assert np.count_nonzero(raw.any(axis=1)) == 1


def test_cnot_direction():
    """cx 0 1 sobre |01> (indice 1) -> |11> (indice 3); sobre |10> no hace nada."""
    raw, _, _ = run_c(2, ["x", 0, "cx", 0, 1])
    assert raw[3, 0] == 16384
    raw, _, _ = run_c(2, ["x", 1, "cx", 0, 1])
    assert raw[2, 0] == 16384


def test_reference_matches_c_order():
    """La referencia NumPy usa el mismo orden que el C (incluso con puertas asimetricas)."""
    circ = [("h", 0), ("x", 2), ("cx", 0, 2), ("z", 2), ("h", 1)]
    _, c, _ = run_c(3, ref.circuit_args(circ))
    assert np.abs(c - ref.run(3, circ)).max() < AMP_TOL


# ----------------------------------------------------------------------- norma
@pytest.mark.parametrize("name", KNOWN)
def test_norm_known_circuits(name):
    n, circ = KNOWN[name]
    _, c, _ = run_c(n, ref.circuit_args(circ))
    assert abs(np.linalg.norm(c) - 1) < NORM_TOL


@pytest.mark.parametrize("name", KNOWN)
def test_amplitudes_match_numpy(name):
    n, circ = KNOWN[name]
    _, c, _ = run_c(n, ref.circuit_args(circ))
    np.testing.assert_allclose(c, ref.run(n, circ), atol=KNOWN_TOL, rtol=0)


def test_ghz6_structure():
    _, c, _ = run_c(6, ref.circuit_args(KNOWN["ghz6"][1]))
    p = probs(c)
    assert p[0] == pytest.approx(0.5, abs=0.01) and p[63] == pytest.approx(0.5, abs=0.01)
    assert p[1:63].max() < 1e-3


# ------------------------------------------------------------------- aleatorios
def test_random_circuits():
    rng = np.random.default_rng(2024)
    for k in range(50):
        n = int(rng.integers(1, 7))
        circ = random_circuit(rng, n, int(rng.integers(1, 41)))
        _, c, _ = run_c(n, ref.circuit_args(circ))
        r = ref.run(n, circ)
        ctx = f"circuito #{k} n={n}: {circ}"
        assert abs(np.linalg.norm(c) - 1) < NORM_TOL, ctx
        assert ref.fidelity(c, r) > FID_MIN, ctx
        assert np.abs(c - r).max() < AMP_TOL, ctx


# --------------------------------------------------------------------- medidas
@pytest.mark.parametrize("seed", range(1, 21))
def test_bell_collapse(seed):
    raw, c, meas = run_c(2, ["h", 0, "cx", 0, 1, "m", 0], seed=seed)
    (q, res), = meas
    assert q == 0 and res in (0, 1)
    expected = 3 if res else 0
    assert abs(np.linalg.norm(c) - 1) < NORM_TOL
    assert probs(c)[expected] > 0.99
    np.testing.assert_allclose(np.delete(c, expected), 0, atol=1 / ONE)


def test_bell_collapse_both_outcomes_happen():
    res = {run_c(2, ["h", 0, "cx", 0, 1, "m", 0], seed=s)[2][0][1] for s in range(1, 21)}
    assert res == {0, 1}


@pytest.mark.parametrize("circ,q,res", [
    ([], 0, 0),                    # |000>, medir q0
    ([("x", 2)], 2, 1),            # |100>, medir q2
    ([("x", 1)], 0, 0),            # q0 definido, otro qubit en |1>
])
def test_measure_deterministic_keeps_norm(circ, q, res):
    """Medir un qubit ya definido no debe cambiar el estado (norma 1)."""
    raw, c, meas = run_c(3, ref.circuit_args(circ) + ["m", q])
    assert meas == [(q, res)]
    np.testing.assert_allclose(c, ref.run(3, circ), atol=AMP_TOL, rtol=0)
    assert abs(np.linalg.norm(c) - 1) < NORM_TOL


GROVER3 = [a for q in range(3) for a in (("h", q),)] + [("o", 5), ("d",), ("o", 5), ("d",)]


@pytest.mark.parametrize("q", range(3))
@pytest.mark.parametrize("seed", range(1, 9))
def test_measure_nonuniform_state_matches_numpy(q, seed):
    """Estado no estabilizador (Grover 3q, p(5)=0.945): tras medir, el estado debe
    ser la proyeccion renormalizada de NumPy para el resultado obtenido."""
    _, c, meas = run_c(3, ref.circuit_args(GROVER3) + ["m", q], seed=seed)
    (_, res), = meas
    expected, _ = ref.project(ref.run(3, GROVER3), 3, q, res)
    assert abs(np.linalg.norm(c) - 1) < NORM_TOL
    np.testing.assert_allclose(c, expected, atol=AMP_TOL, rtol=0)


def test_measure_two_qubits_sequence_ghz():
    """Medir todos los qubits de un GHZ: los resultados deben coincidir y la norma ser 1."""
    ghz = KNOWN["ghz6"][1]
    for seed in range(1, 11):
        _, c, meas = run_c(6, ref.circuit_args(ghz) + [a for q in range(6) for a in ("m", q)], seed=seed)
        bits = {r for _, r in meas}
        assert len(bits) == 1, meas
        assert abs(np.linalg.norm(c) - 1) < NORM_TOL


def test_measure_statistics():
    """~50/50 al medir |+> con 400 semillas."""
    ones = sum(run_c(1, ["h", 0, "m", 0], seed=s)[2][0][1] for s in range(1, 401))
    assert 160 <= ones <= 240


# ----------------------------------------------------------------------- Grover
@pytest.mark.parametrize("target", range(4))
def test_grover_2_qubits(target):
    _, c, _ = run_c(2, ["h", 0, "h", 1, "o", target, "d"])
    assert probs(c)[target] >= 0.99


@pytest.mark.parametrize("n,iters", [(3, 2), (4, 3), (5, 4)])
def test_grover_n_qubits_matches_theory(n, iters):
    """Con n>2 Grover no llega al 99% en pocas iteraciones; se compara con
    la probabilidad teorica sin^2((2k+1)asin(1/sqrt(N)))."""
    target = (1 << n) - 2
    args = [a for q in range(n) for a in ("h", q)]
    for _ in range(iters):
        args += ["o", target, "d"]
    _, c, _ = run_c(n, args)
    theory = np.sin((2 * iters + 1) * np.arcsin(1 / np.sqrt(1 << n))) ** 2
    assert probs(c)[target] == pytest.approx(theory, abs=0.03)
    assert probs(c).argmax() == target
