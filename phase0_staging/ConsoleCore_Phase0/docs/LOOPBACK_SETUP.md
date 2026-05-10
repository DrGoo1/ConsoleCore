# Loopback Setup for ConsoleCore

## Goal

Route audio from any Mac DAW into ConsoleCore and return processed audio to the DAW or monitor path.

## Recommended Phase 1 Device Layout

Create a Loopback virtual device named:

`ConsoleCore Input 16ch`

Channels:

- 1/2 Drums
- 3/4 Bass
- 5/6 Guitars
- 7/8 Keys
- 9/10 Vocals
- 11/12 FX
- 13/14 Parallel A
- 15/16 Parallel B

Create a second virtual device named:

`ConsoleCore Return 2ch`

Channels:

- 1/2 processed stereo return

## DAW Routing

In the source DAW:

1. Create stem buses.
2. Assign each stem bus output to a stereo pair on `ConsoleCore Input 16ch`.
3. Disable duplicate master routing to avoid phasing.
4. Create an aux/input return from `ConsoleCore Return 2ch` if the DAW supports it.

## Latency

Phase 1 must measure and compensate roundtrip latency using an impulse/click test.

Do not judge console feel until latency is measured and stable.
