#pragma once
#include <string>
#include <vector>

struct StemRoute
{
    std::string name;
    int inputL = 1;
    int inputR = 2;
    int consoleStemId = 1;
};

struct ReturnRoute
{
    std::string name;
    int outputL = 1;
    int outputR = 2;
};

struct LoopbackRoutingProfile
{
    std::string profileName;
    std::string inputDeviceName;
    std::string outputDeviceName;
    int sampleRate = 48000;
    int bufferSize = 256;
    int reportedRoundtripLatencySamples = 0;
    std::vector<StemRoute> stems;
    std::vector<ReturnRoute> returns;
};