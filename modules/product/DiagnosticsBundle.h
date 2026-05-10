#pragma once
#include <string>
#include <vector>

struct DiagnosticsBundle
{
    std::string appVersion;
    std::vector<std::string> activeModules;
    std::vector<std::string> loadedProfiles;
    float averageCpuLoad = 0.0f;
};