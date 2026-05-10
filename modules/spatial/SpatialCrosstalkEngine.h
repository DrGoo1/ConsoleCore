#pragma once
#include <vector>

class SpatialCrosstalkEngine
{
public:
    void process(std::vector<float*>& channels, int numSamples, float amount);
};