#include "ConsoleCoreSenderProcessor.h"

bool ConsoleCoreSenderProcessor::initialize(int channelId)
{
    stemId = channelId;
    return true;
}

void ConsoleCoreSenderProcessor::process(float* left, float* right, int numSamples)
{
    // Future distributed rail interaction
}