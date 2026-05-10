#pragma once
#include <string>

struct UserProfile
{
    std::string displayName;
    std::string preferredConsole;
    int preferredSampleRate = 48000;
    bool immersiveModeEnabled = false;
};