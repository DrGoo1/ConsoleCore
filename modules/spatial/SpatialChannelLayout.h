#pragma once
#include <string>
#include <vector>

struct SpatialSpeaker
{
    std::string name;
    int channelIndex = 0;
};

struct SpatialChannelLayout
{
    std::string layoutName;
    std::vector<SpatialSpeaker> speakers;
};