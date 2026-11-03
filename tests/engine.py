"""Utilidades para ejecutar build/qsim_run desde Python."""
import os
import pathlib
import subprocess

import numpy as np

ROOT = pathlib.Path(__file__).resolve().parent.parent
# QSIM_BIN permite probar otro binario (p. ej. la copia C++ del firmware)
BIN = pathlib.Path(os.environ.get("QSIM_BIN", ROOT / "build" / "qsim_run"))
ONE = 16384.0  # Q1.14


def run_c(n, args, seed=None):
    """Devuelve (estado crudo int (dim,2), estado complejo normalizado a Q1.14, medidas)."""
    cmd = [str(BIN), str(n)]
    if seed is not None:
        cmd += ["--seed", str(seed)]
    cmd += [str(a) for a in args]
    p = subprocess.run(cmd, capture_output=True, text=True, check=True)
    raw = np.array([[int(v) for v in ln.split()] for ln in p.stdout.splitlines()])
    meas = [(int(ln.split()[1]), int(ln.split()[2]))
            for ln in p.stderr.splitlines() if ln.startswith("M ")]
    return raw, (raw[:, 0] + 1j * raw[:, 1]) / ONE, meas
