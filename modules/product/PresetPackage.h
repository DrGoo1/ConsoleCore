#pragma once
#include <string>
#include <vector>

struct PresetPackage
{
    std::string packageName;
    std::string version;
    std::vector<std::string> includedProfiles;
};