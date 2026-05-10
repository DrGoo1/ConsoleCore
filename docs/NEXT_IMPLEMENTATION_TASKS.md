# Next Implementation Tasks After This Milestone

## 1. Replace placeholder console stage

In `IntegratedAnalogPathEngine.cpp`, replace the placeholder console stage with the existing ConsoleCore:

```cpp
analogPhysics.processBlock(left, right, numSamples, currentSettings.activeStemCount);
```

## 2. Add per-stem tape processors

Current milestone provides one stereo path.

Next:
- allocate one `TapeTrackProcessor` per stem
- store tape profile per stem
- sum after each tape stage into ConsoleCore bus

## 3. Add runtime UI settings

Expose:
- Tape On/Off per stem
- Machine profile
- IPS
- Bias
- Saturation
- Head bump
- Hiss
- Wow/flutter
- Print tape enable

## 4. Add tape contribution to console rail

TapeSharedState should influence:
- rail stress
- low-frequency load
- transformer stress
- crosstalk
- headroom behavior

## 5. Add capture/profile fitting

Connect to existing Milestone D/R capture systems.
