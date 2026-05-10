#include "PerStemTapeRuntime.h"

void PerStemTapeRuntime::prepare(double sampleRate, int maxBlockSize, int stemCount)
{
    processors.resize(stemCount);

    for (auto& p : processors)
        p.prepare(sampleRate, maxBlockSize);

    tapeState.reset();
}

void PerStemTapeRuntime::reset()
{
    for (auto& p : processors)
        p.reset();

    tapeState.reset();
}

void PerStemTapeRuntime::processStem(int stemIndex,
                                     float* left,
                                     float* right,
                                     int numSamples,
                                     const TapeMachineProfile& profile)
{
    if (stemIndex < 0 || stemIndex >= static_cast<int>(processors.size()))
        return;

    processors[stemIndex].processStereo(
        left,
        right,
        numSamples,
        profile,
        tapeState
    );
}
