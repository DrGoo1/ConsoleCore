#include "MicroInstabilityModel.h"

void MicroInstabilityModel::setSeed(std::uint32_t seed)
{
    state = seed == 0 ? 1 : seed;
}

float MicroInstabilityModel::nextDrift(float amount)
{
    state = 1664525u * state + 1013904223u;
    const float u = static_cast<float>((state >> 8) & 0xFFFFFF) / static_cast<float>(0xFFFFFF);
    return (u * 2.0f - 1.0f) * amount;
}