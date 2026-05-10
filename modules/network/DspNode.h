#pragma once
#include <string>

enum class NodeRole
{
    Local,
    Render,
    Analysis,
    GPU,
    Hybrid
};

struct DspNode
{
    std::string nodeId;
    std::string hostname;
    NodeRole role = NodeRole::Local;

    int cpuCores = 0;
    int gpuUnits = 0;

    bool online = false;
};