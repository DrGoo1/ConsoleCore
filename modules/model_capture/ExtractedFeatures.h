#pragma once

struct ExtractedFeatures
{
    float dryRms = 0.0f;
    float wetRms = 0.0f;
    float dryPeak = 0.0f;
    float wetPeak = 0.0f;
    float crestFactor = 0.0f;
    float gainDeltaDb = 0.0f;
    float estimatedThd = 0.0f;
    float estimatedCrosstalkDb = -120.0f;
    float recoveryMs = 0.0f;
    float lowBandWeight = 0.0f;
    float highBandSoftening = 0.0f;
};