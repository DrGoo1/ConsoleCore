#!/usr/bin/env python3
"""Blind listening round 2: renders the reels in docs/BLIND_REELS_ROUND2.md.

    round2.py --mix FILE[:START_SECONDS] --overheads FILE[:START_SECONDS] --out DIR [--bench PATH]

Cuts 15 s from each source, renders every version through ConsoleCore's tape machines, matches
every file (dry included) to -24 LUFS, and writes each reel's versions as A, B, C (D) in a
shuffled order. The answer key goes to DIR/KEY_open_after_listening.txt.
"""

import argparse
import math
import os
import random
import shutil
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from analyze import read_wav, write_wav, REPO  # noqa: E402

import numpy as np  # noqa: E402

SECONDS = 15.0
TARGET_LUFS = -24.0
MACHINES = {
    "ATR": "tape_profiles/machines/transformerless_servo_24track_2inch_1979.json",
    "M56": "tape_profiles/machines/closed_loop_16track_2inch_1970.json",
}
FORMULAS = {
    "1960s": "low_noise_standard_output_1960s",
    "456": "high_output_1975",
    "GP9": "ultra_high_output_1998",
}
# reel -> list of versions: (label, machine, ips, formula, drive dB); machine None = dry
REELS = [
    ("R1_machines_15ips_drive0", [("dry", None, 0, None, 0), ("ATR", "ATR", 15, "456", 0), ("M56", "M56", 15, "456", 0)]),
    ("R2_machines_15ips_drive+3", [("dry", None, 0, None, 0), ("ATR", "ATR", 15, "456", 3), ("M56", "M56", 15, "456", 3)]),
    ("R3_machines_30ips_drive0", [("dry", None, 0, None, 0), ("ATR", "ATR", 30, "456", 0), ("M56", "M56", 30, "456", 0)]),
    ("R4_machines_30ips_drive+3", [("dry", None, 0, None, 0), ("ATR", "ATR", 30, "456", 3), ("M56", "M56", 30, "456", 3)]),
    ("R5_formulas_ATR_15ips_drive+3", [("dry", None, 0, None, 0), ("1960s tape", "ATR", 15, "1960s", 3),
                                       ("456 tape", "ATR", 15, "456", 3), ("GP9 tape", "ATR", 15, "GP9", 3)]),
]


def biquad_gain(b, a, n, fs):
    # magnitude-squared of a biquad at the rfft bins of an n-point transform
    z = np.exp(-1j * 2 * np.pi * np.fft.rfftfreq(n, 1.0 / fs) / fs)
    h = (b[0] + b[1] * z + b[2] * z * z) / (a[0] + a[1] * z + a[2] * z * z)
    return np.abs(h) ** 2


def k_weight_coeffs(fs):
    # ITU-R BS.1770 pre-filter (high shelf) and RLB high-pass, designed for any rate.
    f0, g, q = 1681.974450955533, 3.999843853973347, 0.7071752369554196
    k = math.tan(math.pi * f0 / fs)
    vh = 10 ** (g / 20)
    vb = vh ** 0.4996667741545416
    a0 = 1 + k / q + k * k
    shelf = ([(vh + vb * k / q + k * k) / a0, 2 * (k * k - vh) / a0, (vh - vb * k / q + k * k) / a0],
             [1.0, 2 * (k * k - 1) / a0, (1 - k / q + k * k) / a0])
    f0, q = 38.13547087602444, 0.5003270373238773
    k = math.tan(math.pi * f0 / fs)
    a0 = 1 + k / q + k * k
    hp = ([1.0, -2.0, 1.0], [1.0, 2 * (k * k - 1) / a0, (1 - k / q + k * k) / a0])
    return shelf, hp


def lufs(x, fs):
    # BS.1770 K-weighting applied by magnitude in the frequency domain (phase does not change
    # block power enough to matter here), then the usual 400 ms gated measurement.
    (sb, sa), (hb, ha) = k_weight_coeffs(fs)
    n = 1 << int(math.ceil(math.log2(len(x) + fs)))
    w = np.sqrt(biquad_gain(sb, sa, n, fs) * biquad_gain(hb, ha, n, fs))
    y = np.fft.irfft(np.fft.rfft(x, n, axis=0) * w[:, None], n, axis=0)[:len(x)]
    block, hop = int(0.4 * fs), int(0.1 * fs)
    powers = np.array([np.sum(np.mean(y[a:a + block] ** 2, axis=0)) for a in range(0, len(y) - block + 1, hop)])
    loud = -0.691 + 10 * np.log10(np.maximum(powers, 1e-20))
    p = powers[loud > -70]
    if len(p) == 0:
        return -70.0
    rel = -0.691 + 10 * np.log10(np.mean(p)) - 10
    p = powers[(loud > -70) & (loud > rel)]
    return float(-0.691 + 10 * np.log10(np.mean(p)))


def cut(spec, tmpdir, name):
    path, start = spec, 0.0
    if ":" in spec and spec.rsplit(":", 1)[1].replace(".", "", 1).isdigit():
        path, s = spec.rsplit(":", 1)
        start = float(s)
    fs, x = read_wav(path)
    if x.shape[1] == 1:
        x = np.repeat(x, 2, axis=1)
    x = x[int(start * fs):int((start + SECONDS) * fs), :2].copy()
    k = int(0.02 * fs)
    ramp = (0.5 - 0.5 * np.cos(np.pi * np.arange(k) / k))[:, None]
    x[:k] *= ramp
    x[-k:] *= ramp[::-1]
    out = os.path.join(tmpdir, name + "_dry.wav")
    write_wav(out, x, fs, float32=True)
    return out, fs


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--mix", required=True, help="full mix, FILE or FILE:START_SECONDS")
    ap.add_argument("--overheads", required=True, help="overheads, FILE or FILE:START_SECONDS")
    ap.add_argument("--out", required=True)
    ap.add_argument("--bench", default=os.path.join(REPO, "build-tape", "tape_machine_bench"))
    ap.add_argument("--seed", type=int, default=2)
    a = ap.parse_args()
    if not os.path.exists(a.bench):
        sys.exit("bench not found at %s: run tools/reference_jig/setup_mac.sh first" % a.bench)
    os.makedirs(a.out, exist_ok=True)
    tmp = os.path.join(a.out, ".work")
    os.makedirs(tmp, exist_ok=True)
    rng = random.Random(a.seed)
    key = ["Blind round 2 answer key. Every file is matched to %.0f LUFS. Flutter on, noise off." % TARGET_LUFS, ""]
    for source, spec in (("mix", a.mix), ("overheads", a.overheads)):
        dry, fs = cut(spec, tmp, source)
        for reel, versions in REELS:
            order = versions[:]
            rng.shuffle(order)
            key.append("%s %s:" % (source, reel))
            for letter, (label, machine, ips, formula, drive) in zip("ABCD", order):
                src = dry
                if machine:
                    src = os.path.join(tmp, "%s_%s_%s_%d_%s_%+d.wav" % (source, reel, machine, ips, formula, drive))
                    subprocess.run([a.bench, "render", os.path.join(REPO, MACHINES[machine]), dry, src, "--ips", str(ips),
                                    "--drive", str(drive), "--formula", os.path.join(REPO, "tape_profiles/formulas/%s.json" % FORMULAS[formula]),
                                    "--float"], check=True, stdout=subprocess.DEVNULL)
                _, x = read_wav(src)
                gain = 10 ** ((TARGET_LUFS - lufs(x, fs)) / 20)
                name = "%s_%s_%s.wav" % (source, reel.split("_")[0], letter)
                write_wav(os.path.join(a.out, name), x * gain, fs)
                desc = "dry" if not machine else "%s, %s tape, %d ips, drive %+d dB" % (machine, formula, ips, drive)
                key.append("  %s = %s" % (letter, desc))
            key.append("")
            print("rendered %s %s" % (source, reel))
    shutil.rmtree(tmp, ignore_errors=True)
    with open(os.path.join(a.out, "KEY_open_after_listening.txt"), "w") as f:
        f.write("\n".join(key) + "\n")
    print("done: %s (key in KEY_open_after_listening.txt)" % a.out)


if __name__ == "__main__":
    main()
