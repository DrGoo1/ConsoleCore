#include "TemporalMemoryEngine.h"
#include "DspUtils.h"
#include <algorithm>
#include <cmath>

void TemporalMemoryEngine::prepare(double sampleRate)
{
    fs = sampleRate > 0.0 ? sampleRate : 48000.0;
    reset();
}

void TemporalMemoryEngine::reset()
{
    memory = 0.0f;
}

float TemporalMemoryEngine::processEnergy(float instantaneousEnergy, float attackMs, float releaseMs, int samplesElapsed)
{
    const bool rising = instantaneousEnergy > memory;
    const float sampleCoeff = cc::onePoleCoeff(rising ? attackMs : releaseMs, fs);
    const float coeff = std::pow(sampleCoeff, static_cast<float>(std::max(1, samplesElapsed)));
    memory = coeff * memory + (1.0f - coeff) * instantaneousEnergy;
    return memory;
}