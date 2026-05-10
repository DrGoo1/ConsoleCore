#pragma once
#include <string>
#include <vector>

struct ConsoleStripUiState
{
    int stemId = 0;
    std::string name;
    float peak = 0.0f;
    float rms = 0.0f;
    float railContribution = 0.0f;
    bool mute = false;
    bool solo = false;
};

struct ConsoleUiState
{
    std::string selectedConsoleProfile;
    std::string selectedRoutingProfile;
    float globalRailStress = 0.0f;
    std::vector<ConsoleStripUiState> strips;
};