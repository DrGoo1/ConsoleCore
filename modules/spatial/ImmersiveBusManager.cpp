#include "ImmersiveBusManager.h"

void ImmersiveBusManager::addBus(const ImmersiveBus& bus)
{
    immersiveBuses.push_back(bus);
}

const std::vector<ImmersiveBus>& ImmersiveBusManager::buses() const
{
    return immersiveBuses;
}