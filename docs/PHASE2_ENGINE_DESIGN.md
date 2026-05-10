# ConsoleCore Phase 2 Engine Design

Phase 2 adds the first proprietary topology-aware mix engine.

## Core concept
The engine treats a mix as a connected console topology rather than isolated channel processors. Each configured stem contributes to a shared rail state. The rail state changes headroom, nonlinear response, crosstalk, and makeup behavior.

## Current implementation
- `TopologyTypes.h`: Stem and topology configuration.
- `ConsoleProfileLibrary`: legally-safe generic console profile presets.
- `TopologyAwareMixEngine`: block processor for multichannel input to stereo output.

## Generic profile families
- Modern Ultra-Clean
- British Class-A Discrete
- British VCA Large Format
- American Discrete Op-Amp
- German Tube Broadcast
- German Solid-State Broadcast
- EMI-Inspired Tube Desk
- Flickinger-Inspired Discrete
- Custom Hybrid

These are behavior families, not exact brand clones.

## Next phase
Phase 3 should add measurement tooling, impulse/sweep capture workflows, model calibration files, and a repeatable process for converting public-domain/owned measurements into ConsoleCore profiles.
