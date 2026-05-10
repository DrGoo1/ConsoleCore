#include "MultibandPhysicsRouter.h"
#include <cmath>

BandEnergy MultibandPhysicsRouter::analyze(const float* mono, int numSamples)
{
    BandEnergy e;

    if (!mono || numSamples <= 0)
        return e;

    double lowE = 0.0;
    double highE = 0.0;
    double total = 0.0;

    for (int i = 0; i < numSamples; ++i)
    {
        const float x = mono[i];

        lowState = 0.995f * lowState + 0.005f * x;
        const float high = x - lowState;

        lowE += lowState * lowState;
        highE += high * high;
        total += x * x;
    }

    e.low = static_cast<float>(std::sqrt(lowE / numSamples));
    e.high = static_cast<float>(std::sqrt(highE / numSamples));
    e.mid = static_cast<float>(std::sqrt(total / numSamples)) - 0.5f * (e.low + e.high);

    return e;
}