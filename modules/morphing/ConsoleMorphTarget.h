#pragma once
#include <string>

struct ConsoleMorphTarget
{
    std::string sourceProfile;
    std::string targetProfile;

    float interpolationAmount = 0.0f;
};