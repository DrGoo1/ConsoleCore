#include "ClusterTopology.h"

void ClusterTopology::addNode(const DspNode& node)
{
    clusterNodes.push_back(node);
}

const std::vector<DspNode>& ClusterTopology::nodes() const
{
    return clusterNodes;
}