#pragma once
#include <cstdint>

class MicroInstabilityModel
{
public:
    void setSeed(std::uint32_t seed);
    float nextDrift(float amount);

private:
    std::uint32_t state = 1;
};