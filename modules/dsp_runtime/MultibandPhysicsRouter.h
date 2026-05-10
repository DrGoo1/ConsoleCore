#pragma once
#include <vector>

struct BandEnergy
{
    float low = 0.0f;
    float mid = 0.0f;
    float high = 0.0f;
};

class MultibandPhysicsRouter
{
public:
    BandEnergy analyze(const float* mono, int numSamples);

private:
    float lowState = 0.0f;
};