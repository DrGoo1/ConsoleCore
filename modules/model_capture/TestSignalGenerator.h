#pragma once
#include <vector>

enum class TestSignalType
{
    Sine1k,
    LogSweep,
    Impulse,
    Step,
    BurstTrain,
    PinkishNoise,
    Multitone
};

class TestSignalGenerator
{
public:
    static std::vector<float> generate(TestSignalType type, double sampleRate, double seconds, float amplitude);
};