#include "ProductionDspEngine.h"
#include <cmath>
#include <algorithm>

static float dbToGainLocal(float db)
{
    return std::pow(10.0f, db / 20.0f);
}

void ProductionDspEngine::prepare(double sampleRate, int)
{
    fs = sampleRate > 0.0 ? sampleRate : 48000.0;
    reset();
}

void ProductionDspEngine::reset()
{
    peakAtomic.store(0.0f);
    railStressAtomic.store(0.0f);
}

void ProductionDspEngine::setSettings(const ProductionDspSettings& settings)
{
    currentSettings = settings;
}

ProductionDspTelemetry ProductionDspEngine::getTelemetry() const
{
    ProductionDspTelemetry t;
    t.peak = peakAtomic.load();
    t.railStress = railStressAtomic.load();
    t.latencySamples = 0.0f;
    return t;
}

void ProductionDspEngine::processStereo(float* left, float* right, int numSamples)
{
    if (!left || !right || numSamples <= 0)
        return;

    const float inGain = dbToGainLocal(currentSettings.inputTrimDb);
    const float outGain = dbToGainLocal(currentSettings.outputTrimDb);

    float peak = 0.0f;
    float energy = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float dryL = left[i];
        const float dryR = right[i];

        float l = dryL * inGain;
        float r = dryR * inGain;

        energy += 0.5f * (l * l + r * r);

        if (!currentSettings.bypass)
        {
            const float stress = std::min(1.0f, energy / static_cast<float>(numSamples) * currentSettings.activeStemCount);
            const float gain = 1.0f / (1.0f + 0.2f * stress);

            l = std::tanh(l * (1.0f + 0.4f * stress)) * gain;
            r = std::tanh(r * (1.0f + 0.4f * stress)) * gain;

            if (currentSettings.deltaMonitor)
            {
                l -= dryL;
                r -= dryR;
            }
        }

        left[i] = l * outGain;
        right[i] = r * outGain;

        peak = std::max(peak, std::max(std::abs(left[i]), std::abs(right[i])));
    }

    peakAtomic.store(peak);
    railStressAtomic.store(std::min(1.0f, energy / std::max(1, numSamples)));
}