#include "TemporalMemoryEngine.h"
#include "DspUtils.h"
#include <algorithm>

void TemporalMemoryEngine::prepare(double sampleRate)
{
    fs = sampleRate > 0.0 ? sampleRate : 48000.0;
    reset();
}

void TemporalMemoryEngine::reset()
{
    memory = 0.0f;
}

float TemporalMemoryEngine::processEnergy(float instantaneousEnergy, float attackMs, float releaseMs)
{
    const bool rising = instantaneousEnergy > memory;
    const float coeff = cc::onePoleCoeff(rising ? attackMs : releaseMs, fs);
    memory = coeff * memory + (1.0f - coeff) * instantaneousEnergy;
    return memory;
}