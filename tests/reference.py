"""Simulador de referencia en NumPy (complex128).

Convencion: qubit 0 = bit MENOS significativo del indice (igual que el C:
mask = 1 << qubit). Verificado en test_engine.py::test_qubit_order.
"""
import numpy as np

SQRT1_2 = 1 / np.sqrt(2)


def zero_state(n):
    s = np.zeros(1 << n, dtype=complex)
    s[0] = 1
    return s


def _idx(n, q):
    i = np.arange(1 << n)
    return i[(i >> q) & 1 == 0]


def h(s, n, q):
    i0 = _idx(n, q); i1 = i0 | (1 << q)
    a, b = s[i0].copy(), s[i1].copy()
    s[i0] = (a + b) * SQRT1_2
    s[i1] = (a - b) * SQRT1_2


def x(s, n, q):
    i0 = _idx(n, q); i1 = i0 | (1 << q)
    s[i0], s[i1] = s[i1].copy(), s[i0].copy()


def z(s, n, q):
    i = np.arange(1 << n)
    s[(i >> q) & 1 == 1] *= -1


def cx(s, n, c, t):
    i = np.arange(1 << n)
    i0 = i[((i >> c) & 1 == 1) & ((i >> t) & 1 == 0)]
    i1 = i0 | (1 << t)
    s[i0], s[i1] = s[i1].copy(), s[i0].copy()


def oracle(s, n, k):
    s[k] *= -1


def diffuser(s, n):
    """Igual que apply_diffuser del C: H^n X^n (-1 en |1..1>) X^n H^n."""
    for q in range(n): h(s, n, q)
    for q in range(n): x(s, n, q)
    s[-1] *= -1
    for q in range(n): x(s, n, q)
    for q in range(n): h(s, n, q)


def run(n, circuit):
    """circuit: lista de tuplas ('h',q) ('x',q) ('z',q) ('cx',c,t) ('o',k) ('d',)."""
    s = zero_state(n)
    for op, *args in circuit:
        {"h": h, "x": x, "z": z, "cx": cx, "o": oracle, "d": diffuser}[op](s, n, *args)
    return s


def circuit_args(circuit):
    return [str(t) for g in circuit for t in g]


def fidelity(a, b):
    na, nb = np.linalg.norm(a), np.linalg.norm(b)
    return abs(np.vdot(a, b)) ** 2 / (na * na * nb * nb)


def project(s, n, q, result):
    """Colapso tras medir `result` en el qubit q: proyecta y renormaliza.
    Devuelve (estado nuevo, probabilidad del resultado)."""
    i = np.arange(1 << n)
    keep = ((i >> q) & 1) == result
    p = float(np.sum(np.abs(s[keep]) ** 2))
    out = np.where(keep, s, 0)
    return out / np.sqrt(p), p
