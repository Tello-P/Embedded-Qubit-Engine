"""Mide el error maximo real del motor C frente a NumPy (para fijar tolerancias).

    python3 tests/measure_error.py
"""
import numpy as np

import reference as ref
from engine import run_c
from test_engine import random_circuit, KNOWN


def main():
    worst = {}
    def upd(k, v): worst[k] = max(worst.get(k, 0), v)

    for name, (n, circ) in KNOWN.items():
        _, c, _ = run_c(n, ref.circuit_args(circ))
        r = ref.run(n, circ)
        upd("conocidos: max|dAmp| (LSB)", np.abs(c - r).max() * 16384)
        upd("conocidos: |norma-1|", abs(np.linalg.norm(c) - 1))
    rng = np.random.default_rng(12345)
    for depth in (5, 10, 20, 40, 80):
        for _ in range(100):
            n = int(rng.integers(1, 7))
            circ = random_circuit(rng, n, depth)
            _, c, _ = run_c(n, ref.circuit_args(circ))
            r = ref.run(n, circ)
            upd(f"aleatorio prof={depth}: max|dAmp| (LSB)", np.abs(c - r).max() * 16384)
            upd(f"aleatorio prof={depth}: |norma-1|", abs(np.linalg.norm(c) - 1))
            upd(f"aleatorio prof={depth}: 1-fidelidad", 1 - ref.fidelity(c, r))
    for k, v in worst.items():
        print(f"{k:45s} {v:.6g}")


if __name__ == "__main__":
    main()
