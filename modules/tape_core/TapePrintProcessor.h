#pragma once
#include "TapeTrackProcessor.h"

class TapePrintProcessor
{
public:
    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    void processStereo(float* left, float* right, int numSamples,
                       const TapeMachineProfile& profile,
                       TapeSharedState& sharedState);

private:
    TapeTrackProcessor processor;
};
