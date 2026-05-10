#include "StemBusManager.h"

void StemBusManager::resize(int stemCount)
{
    stemStates.resize(stemCount);

    for (int i = 0; i < stemCount; ++i)
        stemStates[i].stemId = i + 1;
}

StemBusState& StemBusManager::stem(int index)
{
    return stemStates[index];
}

const std::vector<StemBusState>& StemBusManager::stems() const
{
    return stemStates;
}
