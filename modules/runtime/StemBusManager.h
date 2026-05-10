#pragma once
#include <vector>

struct StemBusState
{
    int stemId = 0;
    float peak = 0.0f;
    float railContribution = 0.0f;
    float tapeCompression = 0.0f;
};

class StemBusManager
{
public:
    void resize(int stemCount);
    StemBusState& stem(int index);
    const std::vector<StemBusState>& stems() const;

private:
    std::vector<StemBusState> stemStates;
};
