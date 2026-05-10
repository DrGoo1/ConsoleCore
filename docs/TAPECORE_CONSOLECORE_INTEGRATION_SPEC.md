# TapeCore → ConsoleCore Integration Specification

TapeCore should become `modules/tape_core/` inside ConsoleCore, not a separate isolated app.

Required signal order:

Input Trim
-> Tape Track Stage
-> Console Channel / Analog Physics
-> Console Bus / Shared Rail
-> Print Tape Stage
-> Output Trim

First scope:
- stable DSP path
- deterministic tape profile handling
- shared tape telemetry
- safe saturation/hysteresis/head-bump scaffolding
- print tape stage
- CLI verification

Future work:
- frequency-dependent bias
- real hysteresis loop modeling
- dynamic tape compression
- inter-track crosstalk
- repro/sync head behavior
- capture-driven tape profile fitting
- hardware SHARC/FPGA migration
