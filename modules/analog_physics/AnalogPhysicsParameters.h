#pragma once

struct AnalogPhysicsParameters
{
    float drive = 0.25f;                 // 0..1
    float railStressAmount = 0.35f;      // 0..1
    float railRecoveryMs = 420.0f;       // ms
    float transformerWeight = 0.25f;     // 0..1
    float tapeMemoryAmount = 0.15f;      // 0..1
    float crosstalkAmount = 0.08f;       // 0..1
    float impedanceInteraction = 0.20f;  // 0..1
    float lowFrequencyLoadBias = 0.30f;  // 0..1
    float highFrequencySlewSoftening = 0.12f; // 0..1
    float outputTrimDb = 0.0f;
};