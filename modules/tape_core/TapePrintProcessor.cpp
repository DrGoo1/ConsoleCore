#include "TapePrintProcessor.h"
#include <algorithm>

void TapePrintProcessor::prepare(double sampleRate, int maxBlockSize)
{
    processor.prepare(sampleRate, maxBlockSize);
}

void TapePrintProcessor::reset()
{
    processor.reset();
}

void TapePrintProcessor::processStereo(float* left, float* right, int numSamples,
                                       const TapeMachineProfile& profile,
                                       TapeSharedState& sharedState)
{
    processor.processStereo(left, right, numSamples, profile, sharedState);
    sharedState.printTapeStress = std::max(sharedState.printTapeStress, sharedState.tapeCompression);
}
