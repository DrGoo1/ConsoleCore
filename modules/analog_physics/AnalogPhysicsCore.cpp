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
    processBlock(left, right, numSamples, activeChannels, ChannelLoadSummary{});
}

void AnalogPhysicsCore::processBlock(float* left, float* right, int numSamples, int activeChannels, const ChannelLoadSummary& channelLoad)
{
    if (!left || numSamples <= 0) return;
    if (!right) right = left;

    constexpr int physicsQuantum = 64;
    const float concentration = std::clamp(channelLoad.concentration, 0.0f, 1.0f);
    const float distributionStress = (activeChannels > 1) ? (1.0f - concentration) : 0.0f;
    const float trim = cc::dbToGain(params.outputTrimDb);
    float peak = 0.0f;
    SpectralLoad lastLoad;
    float lastStress = 0.0f;
    float lastGain = 1.0f;

    for (int offset = 0; offset < numSamples; offset += physicsQuantum)
    {
        const int count = std::min(physicsQuantum, numSamples - offset);
        const auto load = spectral.analyzeBlock(left + offset, right + offset, count);
        const float energy = temporalMemory.processEnergy(
            load.broadband, 6.0f, 180.0f + params.tapeMemoryAmount * 900.0f, count);

        rail.update(energy + params.lowFrequencyLoadBias * load.low,
                    activeChannels, params, distributionStress, count);
        const float stress = rail.getRailStress();
        const float gain = headroom.computeGain(stress, load, activeChannels, params);

        for (int i = offset; i < offset + count; ++i)
        {
            float l = cc::sanitize(left[i]);
            float r = cc::sanitize(right[i]);
            l *= (1.0f + 1.5f * params.drive) * gain;
            r *= (1.0f + 1.5f * params.drive) * gain;
            l = transformerL.processSample(l, stress, params);
            r = transformerR.processSample(r, stress, params);
            crosstalk.processStereo(l, r, energy, stress, params);
            l = cc::softClip(l, 0.10f + 0.35f * stress) * trim;
            r = cc::softClip(r, 0.10f + 0.35f * stress) * trim;
            left[i] = cc::sanitize(l);
            right[i] = cc::sanitize(r);
            peak = std::max(peak, std::max(std::abs(left[i]), std::abs(right[i])));
        }
        lastLoad = load;
        lastStress = stress;
        lastGain = gain;
    }

    telemetry.railVoltage = rail.getRailVoltage();
    telemetry.railStress = lastStress;
    telemetry.memory = temporalMemory.getMemory();
    telemetry.headroomGain = lastGain;
    telemetry.peakOutput = peak;
    telemetry.spectralLoad = lastLoad;
    telemetry.channelConcentration = concentration;
    telemetry.channelDistributionStress = distributionStress;
}
