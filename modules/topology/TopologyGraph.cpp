#include "TopologyGraph.h"

void TopologyGraph::addNode(const ConsoleNode& node) {
    nodes.push_back(node);
}

void TopologyGraph::update(float deltaTime) {
    for (auto& n : nodes) {
        n.interaction = n.load * deltaTime;
    }
}