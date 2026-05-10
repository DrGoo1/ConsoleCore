#pragma once
#include <vector>
#include <string>

struct Capability
{
    std::string name;
};

class CapabilityRegistry
{
public:
    void addCapability(const Capability& capability);
    const std::vector<Capability>& capabilities() const;

private:
    std::vector<Capability> supported;
};