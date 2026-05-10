#pragma once
#include <string>

struct VirtualConsoleStrip
{
    int stemId = 0;
    std::string name;

    float trimDb = 0.0f;
    float pan = 0.0f;

    bool mute = false;
    bool solo = false;

    float meterPeak = 0.0f;
    float railStressContribution = 0.0f;
};