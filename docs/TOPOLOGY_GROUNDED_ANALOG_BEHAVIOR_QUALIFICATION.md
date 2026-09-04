# Topology-Grounded Analog Behavior Qualification

## Purpose

ConsoleCore models a console as an interacting analog system, not a collection of independent saturators. The shared-state architecture generalizes the original REAPER/gmem sender/summer concept while remaining host-independent. External behavioral references such as CON041/THS may establish system-behavior targets but are never copied and are not circuit truth.

## Qualification levels

- `behavior_family`: credible console-system behavior with explicit limits on named-hardware claims.
- `topology_faithful`: topology-specific stage graph with circuit/component grounding and measured behavioral constraints.
- `exact_hardware`: strongest claim; requires direct hardware and/or exact schematic evidence at critical stages plus complete system qualification.

## Required console stages

Every promoted profile declares evidence for: sender/line amp, source impedance, summing node, shared rail/supply, bus amp, output stage/transformer, DC/coupling/servo behavior, and crosstalk field. A stage may be declared physically absent only with evidence explaining why.

## Evidence hierarchy

1. direct hardware measurements
2. authoritative hardware specifications or exact schematic values
3. schematic-inferred values
4. component/topology priors
5. measured behavioral references (including trusted commercial emulations)
6. computational priors
7. unverified assumptions

Behavioral references are valuable for system-level triangulation but may not be the sole grounding for a topology-faithful or exact-hardware stage.

## Mandatory behavior matrix

Qualification is fixed-trim and must cover 1, 2, 4, 8, 16, and 24 active channels. RMS normalization per channel count is prohibited. Required domains are level transfer, harmonic-vs-level behavior, IMD, crest/transient behavior, channel-count interaction, LF load/rail stress, recovery/memory, phase/group delay, crosstalk, and shared-state interaction.

## Shared-state proof

The system must demonstrate that equal final summed RMS can produce different console state when channel distribution differs. At minimum, tests compare concentrated energy against distributed energy and verify bounded changes in rail/load state, nonlinear residual, or recovery behavior. This is the native successor to the original gmem-style shared-console concept.

## Promotion boundary

A passing software test suite is necessary but not sufficient. Promotion requires a machine-readable evidence card, complete mandatory behavior coverage, topology-appropriate provenance, deterministic fingerprints, and no unresolved blocker. The qualification report is auditable and fails closed.
