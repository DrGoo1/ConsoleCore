#pragma once

#include <vector>

namespace consolecore
{
struct RoutingPair
{
    int inputChannel = 0;
    int outputChannel = 0;
    float gain = 1.0f;
};

class RoutingMatrix
{
public:
    void clear();
    void addRoute(int inputChannel, int outputChannel, float gain = 1.0f);
    const std::vector<RoutingPair>& getRoutes() const noexcept { return routes; }

private:
    std::vector<RoutingPair> routes;
};
}
