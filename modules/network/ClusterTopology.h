#pragma once
#include "DspNode.h"
#include <vector>

class ClusterTopology
{
public:
    void addNode(const DspNode& node);
    const std::vector<DspNode>& nodes() const;

private:
    std::vector<DspNode> clusterNodes;
};