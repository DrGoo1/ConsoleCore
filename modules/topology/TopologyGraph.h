#pragma once
#include <vector>
#include <string>

struct ConsoleNode {
    std::string name;
    float load = 0.0f;
    float interaction = 0.0f;
};

class TopologyGraph {
public:
    void addNode(const ConsoleNode& node);
    void update(float deltaTime);
    const std::vector<ConsoleNode>& getNodes() const noexcept { return nodes; }
private:
    std::vector<ConsoleNode> nodes;
};