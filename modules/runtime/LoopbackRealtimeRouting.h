#pragma once
#include <string>

struct LoopbackRealtimeRouting
{
    std::string inputDeviceName;
    std::string outputDeviceName;

    int inputChannels = 16;
    int outputChannels = 2;

    int sampleRate = 48000;
    int bufferSize = 256;
};
