#pragma once
#include <vector>
#include <string>

struct PatchPoint
{
    std::string name;
    std::string connection;
};

struct PatchChainDefinition
{
    std::string chainName;
    std::vector<PatchPoint> path;
};