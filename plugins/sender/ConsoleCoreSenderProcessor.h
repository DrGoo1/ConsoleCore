#pragma once
#include <string>

class ConsoleCoreSenderProcessor
{
public:
    bool initialize(int channelId);
    void process(float* left, float* right, int numSamples);

private:
    int stemId = -1;
};