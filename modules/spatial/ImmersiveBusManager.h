#pragma once
#include <vector>
#include <string>

struct ImmersiveBus
{
    std::string name;
    int channelCount = 2;
};

class ImmersiveBusManager
{
public:
    void addBus(const ImmersiveBus& bus);
    const std::vector<ImmersiveBus>& buses() const;

private:
    std::vector<ImmersiveBus> immersiveBuses;
};