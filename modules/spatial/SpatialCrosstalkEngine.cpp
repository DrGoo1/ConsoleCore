#include "SpatialCrosstalkEngine.h"

void SpatialCrosstalkEngine::process(std::vector<float*>& channels, int numSamples, float amount)
{
    if (channels.size() < 2)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        float average = 0.0f;

        for (auto* ch : channels)
            average += ch[i];

        average /= static_cast<float>(channels.size());

        for (auto* ch : channels)
            ch[i] += average * amount * 0.02f;
    }
}