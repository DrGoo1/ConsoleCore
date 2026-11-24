#!/usr/bin/env python3
"""ConsoleCore reference jig: analyse tape-plugin renders and ConsoleCore renders the same way.

    analyze.py consolecore <jig_dir> [--bench PATH]   render renders/dry/* through ConsoleCore's tape models
    analyze.py analyze     <jig_dir>                  measure every render against its dry file, score the models

Layout (built by ChatGPT in Luna, see docs/REFERENCE_JIG.md):

    <jig_dir>/renders/dry/<anything>_<signal>.wav            the unprocessed test signals
    <jig_dir>/renders/<plugin>/<plugin>_<machine>_<speed>_<formula>_<level>_<bias>_<signal>.wav
    <jig_dir>/renders.csv                                     settings, copied into the report

    e.g. renders/P821/P821_Stephens_30ips_FormulaA_+6dB_bias0_02-levelramp.wav

Signals: 01-sweep 02-levelramp 03-levelramp100 04-tone10k 05-imd 06-pink 07-flutter3150
08-silence 09-drums 10-mix. 48 kHz, 0 VU = -18 dBFS, each file starts with 1 s of silence and an
impulse. Nothing about the exact signals is assumed beyond that: every render is compared with the
dry file of the same signal, after aligning the two on the impulse.

Measured from audio only; no plugin code, preset or data is read. Needs Python 3.8+ and numpy.
"""

import argparse
import csv
import json
import math
import os
import shutil
import struct
import subprocess
import sys

try:
    import numpy as np
except ImportError:  # pragma: no cover
    sys.exit("numpy is needed: run tools/reference_jig/setup_mac.sh, or 'python3 -m pip install numpy'")

FS = 48000
VU0_DBFS = -18.0
HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))
SIGNALS = ["01-sweep", "02-levelramp", "03-levelramp100", "04-tone10k", "05-imd", "06-pink",
           "07-flutter3150", "08-silence", "09-drums", "10-mix"]


def db(x):
    return 20.0 * math.log10(max(float(x), 1e-12))


def third_octaves(lo=20.0, hi=20000.0):
    out, k = [], -17
    while True:
        f = 1000.0 * 2.0 ** (k / 3.0)
        if f > hi * 1.01:
            return out
        if f >= lo * 0.99:
            out.append(round(f, 1))
        k += 1


BANDS = third_octaves()


# --------------------------------------------------------------------------------------- WAV I/O

def read_wav(path):
    """Returns (rate, float64 array [frames, channels]). PCM 16/24/32 and float32/64."""
    with open(path, "rb") as f:
        data = f.read()
    if data[:4] != b"RIFF" or data[8:12] != b"WAVE":
        raise ValueError("%s is not a RIFF/WAVE file" % path)
    pos, fmt, raw = 12, None, None
    while pos + 8 <= len(data):
        cid, size = data[pos:pos + 4], struct.unpack("<I", data[pos + 4:pos + 8])[0]
        body = data[pos + 8:pos + 8 + size]
        if cid == b"fmt ":
            tag, ch, rate = struct.unpack("<HHI", body[:8])
            bits = struct.unpack("<H", body[14:16])[0]
            if tag == 0xFFFE and len(body) >= 26:
                tag = struct.unpack("<H", body[24:26])[0]
            fmt = (tag, ch, rate, bits)
        elif cid == b"data":
            raw = data[pos + 8:pos + 8 + size]
        pos += 8 + size + (size & 1)
    if fmt is None or raw is None:
        raise ValueError("%s has no fmt or data chunk" % path)
    tag, ch, rate, bits = fmt
    if tag == 3 and bits == 32:
        x = np.frombuffer(raw[: len(raw) // 4 * 4], dtype="<f4").astype(np.float64)
    elif tag == 3 and bits == 64:
        x = np.frombuffer(raw[: len(raw) // 8 * 8], dtype="<f8")
    elif tag == 1 and bits == 16:
        x = np.frombuffer(raw[: len(raw) // 2 * 2], dtype="<i2") / 32768.0
    elif tag == 1 and bits == 24:
        b = np.frombuffer(raw[: len(raw) // 3 * 3], dtype=np.uint8).reshape(-1, 3).astype(np.int32)
        v = b[:, 0] | (b[:, 1] << 8) | (b[:, 2] << 16)
        v = np.where(v >= 1 << 23, v - (1 << 24), v)
        x = v / 8388608.0
    elif tag == 1 and bits == 32:
        x = np.frombuffer(raw[: len(raw) // 4 * 4], dtype="<i4") / 2147483648.0
    else:
        raise ValueError("%s: unsupported WAV format %d/%d bit" % (path, tag, bits))
    frames = len(x) // ch
    return rate, x[: frames * ch].reshape(frames, ch)


def write_wav(path, x, rate=FS, float32=False):
    """x: [frames] or [frames, channels] float. 24-bit PCM, or 32-bit float."""
    x = np.asarray(x, dtype=np.float64)
    if x.ndim == 1:
        x = np.stack([x, x], axis=1)
    frames, ch = x.shape
    if float32:
        payload = x.astype("<f4").tobytes()
        tag, bits = 3, 32
    else:
        v = np.round(np.clip(x, -1.0, 8388607.0 / 8388608.0) * 8388608.0).astype(np.int32).reshape(-1)
        b = np.empty((v.size, 3), dtype=np.uint8)
        b[:, 0] = v & 0xFF
        b[:, 1] = (v >> 8) & 0xFF
        b[:, 2] = (v >> 16) & 0xFF
        payload = b.tobytes()
        tag, bits = 1, 24
    block = ch * bits // 8
    with open(path, "wb") as f:
        f.write(b"RIFF" + struct.pack("<I", 36 + len(payload)) + b"WAVEfmt ")
        f.write(struct.pack("<IHHIIHH", 16, tag, ch, rate, rate * block, block, bits))
        f.write(b"data" + struct.pack("<I", len(payload)) + payload)



def amp_at(y, f, half=None):
    """Amplitude of the component near f. Energy is summed over a band around f, so a tone that
    wanders with wow and flutter is measured whole rather than losing level at high frequencies."""
    n = len(y)
    if n < 16:
        return 0.0
    w = np.hanning(n)
    spec = np.fft.rfft(y * w)
    freqs = np.fft.rfftfreq(n, 1.0 / FS)
    if half is None:
        half = max(10.0, 0.01 * f)
    sel = (freqs >= f - half) & (freqs <= f + half)
    # Parseval for a windowed sine of amplitude A: one-sided sum of |X|^2 over its lobe = N A^2 sum(w^2) / 4
    energy = np.sum(np.abs(spec[sel]) ** 2)
    return math.sqrt(4.0 * energy / (n * np.sum(w ** 2)))


def nearest(freq_list, f):
    return min(range(len(freq_list)), key=lambda i: abs(math.log(freq_list[i] / f)))



def nearest(freq_list, f):
    return min(range(len(freq_list)), key=lambda i: abs(math.log(freq_list[i] / f)))


# ------------------------------------------------------------------------------- names and files

def signal_of(path):
    stem = os.path.splitext(os.path.basename(path))[0]
    for sig in SIGNALS:
        if stem.endswith(sig):
            return sig
    return None


def parse_name(path):
    """plugin_machine_speed_formula_level_bias_signal; the machine may itself contain underscores."""
    stem = os.path.splitext(os.path.basename(path))[0]
    parts = stem.split("_")
    info = {"file": path, "signal": signal_of(path), "plugin": parts[0]}
    if len(parts) >= 7:
        info.update({"machine": "_".join(parts[1:-5]), "speed": parts[-5], "formula": parts[-4],
                     "level": parts[-3], "bias": parts[-2]})
    else:
        info.update({"machine": "", "speed": "", "formula": "", "level": "", "bias": ""})
    info["setting"] = "_".join(parts[:-1]) if len(parts) > 1 else stem
    return info


def level_db(text):
    try:
        return float(str(text).lower().replace("db", "").replace("vu", ""))
    except ValueError:
        return 0.0


def speed_ips(text):
    try:
        return float(str(text).lower().replace("ips", ""))
    except ValueError:
        return None


# ----------------------------------------------------------------------------------- alignment

def impulse_index(x):
    """Index of the alignment impulse: the largest sample between 0.5 s and 2.0 s."""
    a, b = int(0.5 * FS), min(int(2.0 * FS), len(x))
    if b <= a:
        return 0
    return a + int(np.argmax(np.abs(x[a:b])))


def aligned(dry, wet):
    """Both trimmed so the impulse sits at the same index, cut to the same length, impulse removed."""
    i, j = impulse_index(dry), impulse_index(wet)
    start = 0.1 * FS
    d = dry[i + int(start):]
    w = wet[j + int(start):]
    n = min(len(d), len(w))
    return d[:n], w[:n], j - i


# --------------------------------------------------------------------------------- measurements

def band_levels(y, n=8192):
    if len(y) < n:
        n = 1 << int(math.log2(max(len(y), 512)))
    w = np.hanning(n)
    acc = np.zeros(n // 2 + 1)
    frames = 0
    for a in range(0, max(len(y) - n, 0) + 1, n // 2):
        acc += np.abs(np.fft.rfft(y[a:a + n] * w)) ** 2
        frames += 1
    acc /= max(frames, 1)
    f = np.fft.rfftfreq(n, 1.0 / FS)
    out = []
    for fc in BANDS:
        sel = (f >= fc / 2 ** (1 / 6)) & (f < fc * 2 ** (1 / 6))
        out.append(10 * math.log10(max(float(np.sum(acc[sel])), 1e-30)))
    return out


def sweep_response(d, w):
    """Transfer function out/in, averaged into third-octave bands, re its value at 1 kHz."""
    n = len(d)
    D, W = np.fft.rfft(d), np.fft.rfft(w)
    f = np.fft.rfftfreq(n, 1.0 / FS)
    out = []
    for fc in BANDS:
        sel = (f >= fc / 2 ** (1 / 6)) & (f < fc * 2 ** (1 / 6))
        pd, pw = float(np.sum(np.abs(D[sel]) ** 2)), float(np.sum(np.abs(W[sel]) ** 2))
        out.append(10 * math.log10(max(pw, 1e-30) / max(pd, 1e-30)) if pd > 1e-20 else float("nan"))
    ref = out[nearest(BANDS, 1000.0)]
    return [v - ref for v in out], ref


def peak_freq(x, lo=20.0, hi=22000.0):
    spec = np.abs(np.fft.rfft(x * np.hanning(len(x))))
    f = np.fft.rfftfreq(len(x), 1.0 / FS)
    sel = (f >= lo) & (f <= hi)
    return float(f[sel][np.argmax(spec[sel])])


def level_ramp(d, w, frame_s):
    """Per frame: input level (VU), gain (dB), H2 and H3 re fundamental (dB). Works for stepped or
    continuous ramps because it only follows what the dry file does."""
    f0 = peak_freq(d, 30.0, 15000.0)
    n = int(frame_s * FS)
    rows = []
    for a in range(0, len(d) - n, n // 2):
        dd, ww = d[a:a + n], w[a:a + n]
        a_in = amp_at(dd, f0)
        if a_in < 10 ** ((VU0_DBFS - 45) / 20):
            continue
        a_out = amp_at(ww, f0)
        row = {"vu": db(a_in) - VU0_DBFS, "gain": db(a_out) - db(a_in)}
        row["h2"] = db(amp_at(ww, 2 * f0) / max(a_out, 1e-12)) if 2 * f0 < FS / 2 else float("nan")
        row["h3"] = db(amp_at(ww, 3 * f0) / max(a_out, 1e-12)) if 3 * f0 < FS / 2 else float("nan")
        rows.append(row)
    # 1 dB bins
    bins = {}
    for r in rows:
        bins.setdefault(int(round(r["vu"])), []).append(r)
    curve = []
    for vu in sorted(bins):
        rs = bins[vu]
        curve.append({"vu": vu, "gain": float(np.median([r["gain"] for r in rs])),
                      "h2": float(np.median([r["h2"] for r in rs])), "h3": float(np.median([r["h3"] for r in rs]))})
    if curve:
        lowest = [c["gain"] for c in curve if c["vu"] <= curve[0]["vu"] + 6]
        ref = float(np.median(lowest))
        for c in curve:
            c["compression"] = c["gain"] - ref
    return {"freq": f0, "curve": curve}


def at_vu(curve, vu, key):
    best = None
    for c in curve:
        if best is None or abs(c["vu"] - vu) < abs(best["vu"] - vu):
            best = c
    if best is None or abs(best["vu"] - vu) > 1.5:
        return float("nan")
    return best[key]


def imd(d, w):
    spec = np.abs(np.fft.rfft(d * np.hanning(len(d))))
    f = np.fft.rfftfreq(len(d), 1.0 / FS)
    order = np.argsort(spec)[::-1]
    tones = []
    for k in order:
        if all(abs(f[k] - t) > 30 for t in tones):
            tones.append(float(f[k]))
        if len(tones) == 2:
            break
    lo, hi = sorted(tones)
    if hi / max(lo, 1.0) > 8:        # SMPTE-style: low tone modulating a high one
        narrow = min(15.0, lo / 3.0)
        carrier = amp_at(w, hi, narrow)
        side = math.sqrt(sum(amp_at(w, hi + k * lo, narrow) ** 2 for k in (-2, -1, 1, 2)))
        kind = "smpte"
    else:                            # twin-tone: difference product
        narrow = min(15.0, (hi - lo) / 3.0)
        carrier = amp_at(w, hi, narrow)
        side = amp_at(w, hi - lo, narrow)
        kind = "difference"
    return {"tones": [lo, hi], "kind": kind, "percent": 100.0 * side / max(carrier, 1e-12)}


def flutter(w):
    f0 = peak_freq(w, 1000.0, 6000.0)
    y = w[int(0.5 * FS):]
    zc = np.where((y[:-1] <= 0) & (y[1:] > 0))[0]
    if len(zc) < 200:
        return float("nan")
    frac = zc + y[zc] / (y[zc] - y[zc + 1])
    freq = FS / np.diff(frac)
    keep = np.abs(freq - np.median(freq)) < 0.05 * np.median(freq)
    freq = freq[keep]
    dev = (freq - np.mean(freq)) / np.mean(freq) * 100.0
    k = max(1, int(round(f0 / 600.0)))   # smooth to about 300 Hz bandwidth
    dev = np.convolve(dev, np.ones(k) / k, mode="valid")
    return float(np.sqrt(np.mean(dev ** 2)))


def measure(sig, d, w):
    if sig == "01-sweep":
        resp, ref = sweep_response(d, w)
        return {"response_db": resp, "gain_1k_db": ref}
    if sig in ("02-levelramp", "03-levelramp100"):
        return level_ramp(d, w, 0.1 if sig == "02-levelramp" else 0.2)
    if sig == "04-tone10k":
        return level_ramp(d, w, 0.05)
    if sig == "05-imd":
        return imd(d, w)
    if sig in ("06-pink", "09-drums", "10-mix"):
        bd, bw = band_levels(d), band_levels(w)
        rd = 10 * math.log10(max(float(np.mean(d ** 2)), 1e-20))
        rw = 10 * math.log10(max(float(np.mean(w ** 2)), 1e-20))
        crest_d = db(np.max(np.abs(d))) - 10 * math.log10(max(float(np.mean(d ** 2)), 1e-20))
        crest_w = db(np.max(np.abs(w))) - 10 * math.log10(max(float(np.mean(w ** 2)), 1e-20))
        diff = [b - a for a, b in zip(bd, bw)]
        mid = float(np.mean([diff[i] for i, fc in enumerate(BANDS) if 200 <= fc <= 2000]))
        return {"spectrum_change_db": [v - mid for v in diff], "gain_db": rw - rd, "crest_change_db": crest_w - crest_d}
    if sig == "07-flutter3150":
        return {"flutter_rms_percent": flutter(w)}
    if sig == "08-silence":
        y = w[int(0.5 * FS):]
        return {"noise_dbfs": 10 * math.log10(max(float(np.mean(y ** 2)), 1e-20))}
    return {}


def headline(m):
    """One row of numbers per setting, from whatever signals it has."""
    s = {}
    if "01-sweep" in m:
        r = m["01-sweep"]["response_db"]
        for f in (30, 50, 100, 10000, 15000, 20000):
            s["resp_%dHz_db" % f] = r[nearest(BANDS, f)]
    if "02-levelramp" in m:
        c = m["02-levelramp"]["curve"]
        s["hd3_at_0vu_percent"] = 100 * 10 ** (at_vu(c, 0, "h3") / 20)
        s["hd2_at_0vu_percent"] = 100 * 10 ** (at_vu(c, 0, "h2") / 20)
        s["h2_minus_h3_at_+6vu_db"] = at_vu(c, 6, "h2") - at_vu(c, 6, "h3")
        s["comp_1k_+6vu_db"] = at_vu(c, 6, "compression")
        s["comp_1k_+12vu_db"] = at_vu(c, 12, "compression")
        mol = float("nan")
        for x in c:
            if 100 * 10 ** (x["h3"] / 20) >= 3.0 and x["vu"] > -10:
                mol = x["vu"]
                break
        s["vu_at_3pct_hd3"] = mol
    if "03-levelramp100" in m:
        s["comp_100hz_+12vu_db"] = at_vu(m["03-levelramp100"]["curve"], 12, "compression")
    if "04-tone10k" in m:
        c = m["04-tone10k"]["curve"]
        s["comp_10k_+6vu_db"] = at_vu(c, 6, "compression") if c else float("nan")
    if "05-imd" in m:
        s["imd_percent"] = m["05-imd"]["percent"]
    if "06-pink" in m:
        sc = m["06-pink"]["spectrum_change_db"]
        s["pink_8to16k_change_db"] = float(np.mean([sc[i] for i, fc in enumerate(BANDS) if 8000 <= fc <= 16000]))
        s["pink_crest_change_db"] = m["06-pink"]["crest_change_db"]
    if "07-flutter3150" in m:
        s["flutter_rms_percent"] = m["07-flutter3150"]["flutter_rms_percent"]
    if "08-silence" in m:
        s["noise_db_re_0vu"] = m["08-silence"]["noise_dbfs"] - (VU0_DBFS - 3.0103)
    for sig in ("09-drums", "10-mix"):
        if sig in m:
            sc = m[sig]["spectrum_change_db"]
            s["%s_hf_change_db" % sig[3:]] = float(np.mean([sc[i] for i, fc in enumerate(BANDS) if 8000 <= fc <= 16000]))
            s["%s_crest_change_db" % sig[3:]] = m[sig]["crest_change_db"]
    return s


# ------------------------------------------------------------------------------------- scoring

def rms(values):
    v = [x for x in values if math.isfinite(x)]
    return math.sqrt(sum(x * x for x in v) / len(v)) if v else float("nan")


def curve_err(a, b, key, lo_vu=-6, floor=None):
    out = []
    for x in a:
        if x["vu"] < lo_vu:
            continue
        y = at_vu(b, x["vu"], key)
        if not math.isfinite(y) or not math.isfinite(x[key]):
            continue
        p, q = x[key], y
        if floor is not None:
            if max(p, q) < floor:
                continue
            p, q = max(p, floor), max(q, floor)
        out.append(p - q)
    return rms(out)


def score(ref, mod):
    parts = {}
    if "01-sweep" in ref and "01-sweep" in mod:
        parts["response_db"] = rms([a - b for a, b, fc in zip(ref["01-sweep"]["response_db"], mod["01-sweep"]["response_db"], BANDS) if 25 <= fc <= 18000])
    if "02-levelramp" in ref and "02-levelramp" in mod:
        parts["compression_db"] = curve_err(ref["02-levelramp"]["curve"], mod["02-levelramp"]["curve"], "compression")
        parts["harmonics_db"] = 0.5 * (curve_err(ref["02-levelramp"]["curve"], mod["02-levelramp"]["curve"], "h3", -20, -70)
                                       + curve_err(ref["02-levelramp"]["curve"], mod["02-levelramp"]["curve"], "h2", -20, -70))
    if "03-levelramp100" in ref and "03-levelramp100" in mod:
        parts["compression_100hz_db"] = curve_err(ref["03-levelramp100"]["curve"], mod["03-levelramp100"]["curve"], "compression")
    for sig in ("06-pink", "09-drums", "10-mix"):
        if sig in ref and sig in mod:
            parts["%s_spectrum_db" % sig[3:]] = rms([a - b for a, b, fc in zip(ref[sig]["spectrum_change_db"], mod[sig]["spectrum_change_db"], BANDS) if 30 <= fc <= 16000])
    if "07-flutter3150" in ref and "07-flutter3150" in mod:
        parts["flutter_percent"] = abs(ref["07-flutter3150"]["flutter_rms_percent"] - mod["07-flutter3150"]["flutter_rms_percent"])
    weights = {"harmonics_db": 0.15, "flutter_percent": 20.0}
    vals = [weights.get(k, 1.0) * v for k, v in parts.items() if math.isfinite(v)]
    parts["total"] = sum(vals) / max(len(vals), 1) * 4.0 if vals else float("nan")
    return parts


# ------------------------------------------------------------------------------------ commands

def collect(jig):
    rdir = os.path.join(jig, "renders")
    dry, wet = {}, []
    for root, _, names in os.walk(rdir):
        for n in sorted(names):
            if not n.lower().endswith(".wav"):
                continue
            p = os.path.join(root, n)
            sig = signal_of(p)
            if sig is None:
                continue
            if os.path.basename(root).lower() == "dry":
                dry[sig] = p
            else:
                wet.append(p)
    return dry, wet


def load_mono(path):
    rate, x = read_wav(path)
    if rate != FS:
        raise ValueError("%d Hz; the jig is 48 kHz" % rate)
    return x[:, 0]


def analyze(jig):
    dry, wet = collect(jig)
    if not dry:
        raise SystemExit("no dry files in %s/renders/dry (names must end in the signal, e.g. _02-levelramp.wav)" % jig)
    dry_audio = {s: load_mono(p) for s, p in dry.items()}
    settings = {}
    for p in wet:
        info = parse_name(p)
        sig = info["signal"]
        if sig not in dry_audio:
            print("skip %s: no dry %s" % (os.path.relpath(p, jig), sig))
            continue
        try:
            d, w, lat = aligned(dry_audio[sig], load_mono(p))
        except ValueError as e:
            print("skip %s: %s" % (os.path.relpath(p, jig), e))
            continue
        st = settings.setdefault(info["setting"], {"info": {k: v for k, v in info.items() if k not in ("file", "signal")}, "m": {}, "latency": {}})
        st["m"][sig] = measure(sig, d, w)
        st["latency"][sig] = lat
    if not settings:
        raise SystemExit("no processed renders found next to renders/dry")
    out = os.path.join(jig, "report")
    os.makedirs(out, exist_ok=True)
    rows = []
    for key in sorted(settings):
        st = settings[key]
        row = dict(st["info"])
        row["signals"] = len(st["m"])
        row["latency_samples"] = int(np.median(list(st["latency"].values())))
        row.update({k: (round(v, 3) if math.isfinite(v) else "") for k, v in headline(st["m"]).items()})
        rows.append(row)
        print("measured %-70s %d signals" % (key, len(st["m"])))
    cols = []
    for r in rows:
        for k in r:
            if k not in cols:
                cols.append(k)
    with open(os.path.join(out, "summary.csv"), "w", newline="") as f:
        wr = csv.DictWriter(f, fieldnames=cols)
        wr.writeheader()
        wr.writerows(rows)
    with open(os.path.join(out, "metrics.json"), "w") as f:
        json.dump({k: v["m"] for k, v in settings.items()}, f)
    if os.path.exists(os.path.join(jig, "renders.csv")):
        shutil.copy(os.path.join(jig, "renders.csv"), os.path.join(out, "renders.csv"))

    # Each reference setting against every ConsoleCore setting at the same speed and level.
    fmap = {}
    if os.path.exists(os.path.join(jig, "formula_map.json")):
        with open(os.path.join(jig, "formula_map.json")) as f:
            fmap = json.load(f)
    refs = {k: v for k, v in settings.items() if not v["info"]["plugin"].lower().startswith("consolecore")}
    mods = {k: v for k, v in settings.items() if v["info"]["plugin"].lower().startswith("consolecore")}
    scores = []
    for rk, rv in refs.items():
        ri = rv["info"]
        for mk, mv in mods.items():
            mi = mv["info"]
            if speed_ips(ri["speed"]) and speed_ips(mi["speed"]) != speed_ips(ri["speed"]):
                continue
            if level_db(mi["level"]) != level_db(ri["level"]):
                continue
            if mi["bias"] != "bias0" and mi["bias"] != ri["bias"]:
                continue
            want = fmap.get("%s_%s" % (ri["plugin"], ri["formula"])) or fmap.get(ri["formula"])
            if want and mi["formula"] != want:
                continue
            sc = score(rv["m"], mv["m"])
            row = {"reference": rk, "model": mk}
            row.update({k: round(v, 3) for k, v in sc.items() if math.isfinite(v)})
            scores.append(row)
    scores.sort(key=lambda r: (r["reference"], r.get("total", 1e9)))
    if scores:
        cols = ["reference", "model"]
        for r in scores:
            for k in r:
                if k not in cols:
                    cols.append(k)
        with open(os.path.join(out, "scores.csv"), "w", newline="") as f:
            wr = csv.DictWriter(f, fieldnames=cols)
            wr.writeheader()
            wr.writerows(scores)
    write_report(out, rows, scores)
    print("report: %s" % os.path.join(out, "REPORT.md"))


def write_report(out, rows, scores):
    cols = ["plugin", "machine", "speed", "formula", "level", "bias", "hd3_at_0vu_percent", "vu_at_3pct_hd3",
            "comp_1k_+12vu_db", "h2_minus_h3_at_+6vu_db", "resp_50Hz_db", "resp_15000Hz_db", "pink_8to16k_change_db",
            "mix_hf_change_db", "mix_crest_change_db", "flutter_rms_percent", "noise_db_re_0vu"]
    lines = ["# Reference jig report", "",
             "Measured from the rendered files against the dry signals. Levels re 0 VU = -18 dBFS.", "",
             "| " + " | ".join(cols) + " |", "|" + "---|" * len(cols)]
    for r in rows:
        lines.append("| " + " | ".join(("%.2f" % r[c]) if isinstance(r.get(c), float) else str(r.get(c, "")) for c in cols) + " |")
    if scores:
        lines += ["", "## Closest ConsoleCore setting per reference (lower total is closer)", "",
                  "| reference | closest ConsoleCore | total | response | compression | harmonics | pink | mix |", "|---|---|---|---|---|---|---|---|"]
        seen = set()
        for r in scores:
            if r["reference"] in seen:
                continue
            seen.add(r["reference"])
            g = lambda k: ("%.2f" % r[k]) if k in r else ""
            lines.append("| %s | %s | %s | %s | %s | %s | %s | %s |" % (r["reference"], r["model"], g("total"), g("response_db"),
                         g("compression_db"), g("harmonics_db"), g("pink_spectrum_db"), g("mix_spectrum_db")))
    with open(os.path.join(out, "REPORT.md"), "w") as f:
        f.write("\n".join(lines) + "\n")


CC_MACHINES = {
    "ATR": "tape_profiles/machines/transformerless_servo_24track_2inch_1979.json",
    "M56": "tape_profiles/machines/closed_loop_16track_2inch_1970.json",
}
CC_FORMULAS = {"1960s-standard": "low_noise_standard_output_1960s", "1975-456type": "high_output_1975", "1998-GP9type": "ultra_high_output_1998"}


def consolecore(jig, bench, levels=(0, 3, 6), speeds=(15, 30)):
    if not os.path.exists(bench):
        raise SystemExit("ConsoleCore bench not found at %s (setup_mac.sh builds it)" % bench)
    dry, _ = collect(jig)
    if not dry:
        raise SystemExit("put the dry signals in %s/renders/dry first" % jig)
    out_dir = os.path.join(jig, "renders", "ConsoleCore")
    os.makedirs(out_dir, exist_ok=True)
    jobs = [(m, s, f, l, 0) for m in CC_MACHINES for s in speeds for f in CC_FORMULAS for l in levels]
    jobs += [(m, 15, "1975-456type", 0, b) for m in CC_MACHINES for b in (-2, 2)]
    count = 0
    for machine, ips, ftag, level, bias in jobs:
        for sig, path in sorted(dry.items()):
            name = "ConsoleCore_%s_%dips_%s_%+ddB_bias%s_%s.wav" % (machine, ips, ftag, level, ("%+d" % bias) if bias else "0", sig)
            dst = os.path.join(out_dir, name)
            if os.path.exists(dst):
                continue
            # The input level is applied before the tape, as a plug-in input trim would be; the bench's
            # own drive compensation is undone so the output level follows the input level.
            tmp = dst + ".in.wav"
            rate, x = read_wav(path)
            write_wav(tmp, x * 10 ** (level / 20.0), rate, float32=True)
            cmd = [bench, "render", os.path.join(REPO, CC_MACHINES[machine]), tmp, dst, "--ips", str(ips),
                   "--formula", os.path.join(REPO, "tape_profiles/formulas/%s.json" % CC_FORMULAS[ftag]), "--float"]
            if bias:
                cmd += ["--bias", str(bias)]
            subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL)
            os.remove(tmp)
            count += 1
    print("ConsoleCore: %d new renders in %s" % (count, out_dir))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("command", choices=["consolecore", "analyze"])
    ap.add_argument("jig")
    ap.add_argument("--bench", default=os.path.join(REPO, "build-tape", "tape_machine_bench"))
    a = ap.parse_args()
    jig = os.path.abspath(a.jig)
    if a.command == "consolecore":
        consolecore(jig, a.bench)
    else:
        analyze(jig)


if __name__ == "__main__":
    main()
