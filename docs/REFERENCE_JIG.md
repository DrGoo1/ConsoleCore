# Reference jig: measuring tape plugins against ConsoleCore

Purpose: fit ConsoleCore's tape models to tape plug-ins Dave owns and trusts (first target the
Pulsar 821 model, then the UAD Ampex, the UAD J-37 and any others), by measuring all of them, and
ConsoleCore, the same way. Everything is measured from rendered audio. No plug-in code, preset or
data is read, copied or shared, and the plug-ins render only on Dave's own licensed Mac.

ChatGPT builds the test signals and the Luna session; this repository holds the analysis
(`tools/reference_jig/analyze.py`) and the ConsoleCore side.

## Folder layout

```
/Volumes/MP 1/Claude_Files/ConsoleCore_reference_jig/
  renders.csv                     settings of every render (copied into the report)
  formula_map.json                optional: which ConsoleCore tape each plug-in tape is scored against
  renders/dry/<any>_<signal>.wav  the unprocessed signals
  renders/<plugin>/<plugin>_<machine>_<speed>_<formula>_<level>_<bias>_<signal>.wav
  renders/ConsoleCore/...         written by the analysis tool, same naming
  report/                         written by the analysis tool
```

Example: `renders/P821/P821_Stephens_30ips_FormulaA_+6dB_bias0_02-levelramp.wav`.
Fields are separated by underscores, so keep underscores out of every field except the machine.
`<level>` is the input level change into the plug-in (for example `+0dB`, `+6dB`); `<bias>` is
`bias0` unless the plug-in's bias control was moved (`bias+2`, `bias-2`, or its own label).

## Signals

48 kHz, 24-bit, 0 VU = -18 dBFS (sine peak). Every file starts with 1 s of silence, then a single
impulse, then the signal; the analysis aligns each render to its dry file on that impulse, so plug-in
latency does not matter. The tool does not assume exact frequencies, lengths or level steps: it
follows what the dry file does.

| Signal | What it should be | What is measured |
|---|---|---|
| `01-sweep` | log sine sweep 20 Hz-20 kHz at about -20 VU | frequency response (third-octave), re 1 kHz |
| `02-levelramp` | 1 kHz, stepped or continuous from about -40 VU to +18 VU | gain vs level (compression), 2nd and 3rd harmonic vs level, level of 3 % third harmonic |
| `03-levelramp100` | the same at 100 Hz | low-frequency compression (head bump, NAB bass, transformers) |
| `04-tone10k` | 10 kHz ramp or steps around -20 to +12 VU | treble compression (tape's high-frequency headroom) |
| `05-imd` | 60 Hz + 7 kHz at 4:1 (or two close high tones) | intermodulation percent |
| `06-pink` | pink noise around 0 VU RMS | spectral change by band, crest change |
| `07-flutter3150` | 3150 Hz at 0 VU, at least 8 s | wow and flutter, rms percent |
| `08-silence` | silence, at least 5 s | noise floor re 0 VU (with hiss on, if testing it) |
| `09-drums`, `10-mix` | Dave's own excerpts | spectral change by band, crest change |

## Commands (on the Mac, from the ConsoleCore checkout)

```
bash tools/reference_jig/setup_mac.sh "/Volumes/MP 1/Claude_Files/ConsoleCore_reference_jig"
```

builds the tape bench, installs numpy into `ConsoleCore_reference_jig/.venv`, and, if the dry
signals are already in `renders/dry`, renders them through ConsoleCore (both machines, the three
formulas, 15 and 30 ips, input level +0/+3/+6 dB, and bias -2/+2 at 15 ips on the 1975 tape).
Then, once the plug-in renders are in:

```
"/Volumes/MP 1/Claude_Files/ConsoleCore_reference_jig/.venv/bin/python" tools/reference_jig/analyze.py analyze "/Volumes/MP 1/Claude_Files/ConsoleCore_reference_jig"
```

## Report

- `report/REPORT.md`: one row per render setting (distortion at 0 VU, level of 3 % third
  harmonic, compression at +12 VU, second vs third harmonic, response at 50 Hz and 15 kHz,
  treble change on pink noise and the mix, flutter, noise), then the closest ConsoleCore setting
  for each reference.
- `report/summary.csv`: the same with every headline figure.
- `report/scores.csv`: every reference against every ConsoleCore setting at the same speed and
  input level, with the error split into response, compression (1 kHz and 100 Hz), harmonics,
  pink and music spectra, and flutter. Lower is closer; 1.0 is roughly "close by ear", but that
  scale is a starting point, not a calibrated one.
- `report/metrics.json`: full curves, for fitting.

To score a plug-in tape only against the nearest ConsoleCore tape, add `formula_map.json`:

```
{ "P821_FormulaA": "1975-456type", "UADAmpex_456": "1975-456type", "UADAmpex_GP9": "1998-GP9type" }
```

(keys are `<plugin>_<formula>` as in the file names, or just `<formula>`; values are
`1960s-standard`, `1975-456type` or `1998-GP9type`). Without it, every formula is scored.

## Limits

- The references are plug-ins, not machines. A close score means ConsoleCore behaves like that
  plug-in at that setting, nothing more.
- Plug-ins with noise or random flutter make the harmonic and noise figures move slightly from run
  to run; turn hiss off unless testing it.
- A plug-in's own calibration control (operating level) shifts everything on the level axis.
  Record its value in `renders.csv`; compare like with like.
