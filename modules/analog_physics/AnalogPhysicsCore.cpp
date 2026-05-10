#include "AnalogPhysicsCore.h"
#include "DspUtils.h"
#include <algorithm>
#include <cmath>

void AnalogPhysicsCore::prepare(double sampleRate, int)
{
    fs = sampleRate > 0.0 ? sampleRate : 48000.0;
    rail.prepare(fs);
    temporalMemory.prepare(fs);
    transformerL.prepare(fs);
    transformerR.prepare(fs);
    spectral.prepare(fs);
    crosstalk.reset();
    reset();
}

void AnalogPhysicsCore::reset()
{
    rail.reset();
    temporalMemory.reset();
    transformerL.reset();
    transformerR.reset();
    spectral.reset();
    crosstalk.reset();
    telemetry = {};
}

void AnalogPhysicsCore::setParameters(const AnalogPhysicsParameters& p)
{
    params = p;
}

void AnalogPhysicsCore::processBlock(float* left, float* right, int numSamples, int activeChannels)
{
    if (!left || numSamples <= 0) return;
    if (!right) right = left;

    const auto load = spectral.analyzeBlock(left, right, numSamples);
    const float energy = temporalMemory.processEnergy(load.broadband, 6.0f, 180.0f + params.tapeMemoryAmount * 900.0f);

    rail.update(energy + params.lowFrequencyLoadBias * load.low, activeChannels, params);
    const float stress = rail.getRailStress();
    const float gain = headroom.computeGain(stress, load, activeChannels, params);
    const float trim = cc::dbToGain(params.outputTrimDb);

    float peak = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float l = cc::sanitize(left[i]);
        float r = cc::sanitize(right[i]);

        l *= (1.0f + 1.5f * params.drive) * gain;
        r *= (1.0f + 1.5f * params.drive) * gain;

        l = transformerL.processSample(l, stress, params);
        r = transformerR.processSample(r, stress, params);

        crosstalk.processStereo(l, r, energy, stress, params);

        // final adaptive soft ceiling
        l = cc::softClip(l, 0.10f + 0.35f * stress) * trim;
        r = cc::softClip(r, 0.10f + 0.35f * stress) * trim;

        left[i] = cc::sanitize(l);
        right[i] = cc::sanitize(r);

        peak = std::max(peak, std::max(std::abs(left[i]), std::abs(right[i])));
    }

    telemetry.railVoltage = rail.getRailVoltage();
    telemetry.railStress = stress;
    telemetry.memory = temporalMemory.getMemory();
    telemetry.headroomGain = gain;
    telemetry.peakOutput = peak;
    telemetry.spectralLoad = load;
}