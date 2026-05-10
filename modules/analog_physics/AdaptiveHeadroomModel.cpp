#include "AdaptiveHeadroomModel.h"
#include "DspUtils.h"
#include <algorithm>
#include <cmath>

float AdaptiveHeadroomModel::computeGain(float railStress, const SpectralLoad& load, int activeChannels, const AnalogPhysicsParameters& params) const
{
    const float channelLoad = std::sqrt(static_cast<float>(std::max(1, activeChannels))) * 0.035f;
    const float spectralBias = params.lowFrequencyLoadBias * load.low + 0.25f * params.highFrequencySlewSoftening * load.high;
    const float reduction = railStress * (0.12f + 0.45f * params.railStressAmount) + channelLoad + 0.12f * spectralBias;
    return std::clamp(1.0f / (1.0f + reduction), 0.45f, 1.05f);
}