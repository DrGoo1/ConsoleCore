#include "CapabilityRegistry.h"

void CapabilityRegistry::addCapability(const Capability& capability)
{
    supported.push_back(capability);
}

const std::vector<Capability>& CapabilityRegistry::capabilities() const
{
    return supported;
}