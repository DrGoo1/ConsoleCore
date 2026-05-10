#pragma once
#include "TapeMachineProfile.h"
#include "TapeSharedState.h"
#include <cstdint>

class TapeTrackProcessor
{
public:
    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    void processStereo(float* left, float* right, int numSamples,
                       const TapeMachineProfile& profile,
                       TapeSharedState& sharedState);

private:
    double fs = 48000.0;
    float hysteresisL = 0.0f;
    float hysteresisR = 0.0f;
    float lowStateL = 0.0f;
    float lowStateR = 0.0f;
    std::uint32_t noiseState = 1;
};
