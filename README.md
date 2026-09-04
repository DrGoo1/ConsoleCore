# ConsoleCore

**Topology-grounded, system-level analog console modeling for modern DAWs.**

ConsoleCore is not designed as a collection of static EQ curves and saturation presets. Its goal is to reproduce the *behavior of a console as an interacting electrical system*: individual channel electronics, summing-node loading, bus amplification, transformers, shared power-supply stress, recovery/memory, crosstalk, tolerances, and the way those mechanisms change as a session becomes larger and more dynamically complex.

The project began with the shared-state lesson demonstrated by REAPER/gmem-style console systems and DocShadrach/THS-style sender/summer behavior: a convincing desk cannot always be represented as independent channel saturators followed by one bus saturator. ConsoleCore generalizes that idea into a host-independent native `SharedAnalogEnvironment`, then adds topology-specific and evidence-grounded modeling.

## Product thesis

A real analog console is a network, not a preset. Two sessions with the same final RMS can stress a desk differently when one signal is concentrated into a few channels and another is distributed across many channels. ConsoleCore therefore models and qualifies both **per-channel nonlinear behavior** and **shared system state**.

The intended signal model is broadly:

`source/load interaction → channel electronics → fader/pan/routing → summing node → shared supply/load environment → bus amplifier → output stage/transformer`

The graph changes where real console topology changes. A transformer-coupled discrete Class-A family must not simply reuse the rail/current assumptions of an IC/VCA large-format desk or a tube broadcast console.

## Evidence hierarchy

ConsoleCore uses an explicit evidence hierarchy instead of assigning undocumented "mojo" coefficients:

1. Direct hardware measurements.
2. Authoritative hardware/service specifications.
3. Exact or reviewed schematic evidence.
4. Schematic/component-derived calculations and simulations.
5. Evidence-bounded topology priors.
6. Trusted commercial emulations and THS-style systems as behavioral references.
7. Listening evaluation after the physical and measurement constraints are satisfied.

Commercial plugins are valuable behavioral benchmarks, but they are never treated as proof of circuit topology. Private schematics and proprietary captures stay outside the public repository; sanitized numerical constraints and provenance records may be committed when appropriate.

## Qualification levels

ConsoleCore separates what a model *sounds like* from what the evidence can legitimately claim:

- **Behavior-family** — measured behavior is consistent with a console family, without claiming exact topology.
- **Topology-inferred / evidence-bounded** — incomplete designs are represented by plausible topology graphs and bounded component ranges, ensemble simulation, confidence scores, and sensitivity analysis. This is research evidence and cannot promote a model by itself.
- **Topology-faithful** — major stages and numeric choices are supported by reviewed circuit/topology evidence plus multidimensional behavioral qualification.
- **Exact-hardware** — a specific implementation has sufficiently strong hardware/spec/schematic evidence across the required stages and operating conditions.

Every production model carries a machine-readable evidence card. Promotion fails closed when required provenance or behavioral domains are missing.

## Evidence-bounded topology inference

Rare and custom consoles may never yield a complete schematic. ConsoleCore does not solve that by inventing a single value. Unknowns are encoded as credible ranges with provenance and confidence, then evaluated as deterministic ensembles.

Sensitivity analysis ranks the unknowns by their influence on audible/system behavior. Research effort is concentrated on high-sensitivity variables such as summing impedance, supply source impedance, output-stage loading, transformer behavior, feedback depth, or quiescent current. Low-impact uncertainty can remain bounded without blocking useful family modeling.

The Flickinger-inspired research model is an example: it remains explicitly low-confidence until stronger evidence is obtained, and its high-sensitivity unknowns are exposed rather than hidden behind a named preset.

## Shared-console qualification

System qualification uses fixed trim and matched program material at **1 / 2 / 4 / 8 / 16 / 24 channels**. Per-count RMS normalization is prohibited because it can hide the very channel-count behavior being measured.

Required domains include level transfer, H2/H3/THD versus level, IMD, crest/transient response, LF load and rail stress, recovery/memory, phase/group delay, crosstalk, fixed-trim channel-count behavior, and concentrated-versus-distributed shared-state interaction.

The matched-energy qualification fixtures are constructed so concentrated and distributed cases sum to effectively the same waveform. Any downstream difference can therefore be attributed to the modeled console system rather than different input material.

## Reference strategy

Where hardware is available, hardware is the primary anchor. Existing Class-A work includes real two-channel transformer/discrete hardware summing measurements and circuit-derived stage evidence. Where a full desk is unavailable, trusted commercial systems such as LUNA console/summing implementations can constrain macroscopic behavior without becoming circuit truth.

This lets ConsoleCore ask a stronger question than "does it null against a plugin?": **does the model produce plausible behavior for the right physical reasons, across level, spectrum, time, channel count, loading, and topology?**

## DSP safeguards discovered through qualification

The qualification program has already identified structural defects that ordinary level-matched A/B testing could conceal. Research paths now enforce host-buffer-independent analog-state timing, order-invariant shared state, unity small-signal nonlinear slope, true fixed-trim qualification, and removal of synthetic DC offsets as a stand-in for component tolerance.

Current legacy coefficients remain explicitly marked as unverified where evidence has not yet replaced them. New topology-sensitive parameters default to neutral values until supported by measurements or bounded circuit evidence.

## Sonic objective

The goal is not maximum coloration. The goal is the characteristic *emergent behavior* of different console families: depth, density, transient handling, overload character, low-frequency authority, harmonic progression, stereo interaction, crosstalk structure, and the subtle change in feel as many channels share real electrical resources.

If successful, those signatures should arise from topology and calibrated system behavior rather than from arbitrary genre labels or fixed "warmth" controls. StudioMind can then select a ConsoleCore family because its physical/sonic behavior suits the production intent while retaining a clear evidence trail for why that model behaves as it does.

## Current status

The topology-grounded qualification framework is active and fail-closed. Existing console families remain research profiles until their evidence cards satisfy the required stages and system-behavior matrix. No model name should be interpreted as an exact-hardware claim unless its qualification level explicitly says so.

See `qualification/` for evidence cards, priors, behavioral matrices, reference policies, and topology-inference tooling. The broader circuit-modeling and validation plans live under `docs/`.

## Build and test

```bash
git clone https://github.com/DrGoo1/ConsoleCore.git
cd ConsoleCore
git submodule update --init --recursive
cmake --preset macos-debug
cmake --build --preset macos-debug
ctest --preset macos-debug -V
```

Research qualification is intentionally separate from production promotion. Passing a development or smoke test is not equivalent to analog-fidelity certification.
