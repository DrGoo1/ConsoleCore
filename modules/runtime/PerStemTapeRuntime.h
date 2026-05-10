#pragma once
#include "../tape_core/TapeTrackProcessor.h"
#include "../tape_core/TapeMachineProfile.h"
#include "../tape_core/TapeSharedState.h"
#include <vector>

class PerStemTapeRuntime
{
public:
    void prepare(double sampleRate, int maxBlockSize, int stemCount);
    void reset();

    void processStem(int stemIndex,
                     float* left,
                     float* right,
                     int numSamples,
                     const TapeMachineProfile& profile);

    const TapeSharedState& sharedState() const { return tapeState; }

private:
    std::vector<TapeTrackProcessor> processors;
    TapeSharedState tapeState;
};
