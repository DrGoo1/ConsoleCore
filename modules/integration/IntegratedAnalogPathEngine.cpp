#include "IntegratedAnalogPathEngine.h"
#include "../tape_core/TapeDspUtils.h"
#include <algorithm>
#include <cmath>

void IntegratedAnalogPathEngine::prepare(double sampleRate, int maxBlockSize)
{
    fs = sampleRate > 0.0 ? sampleRate : 48000.0;
    tapeTrack.prepare(fs, maxBlockSize);
    printTape.prepare(fs, maxBlockSize);
    reset();
}

void IntegratedAnalogPathEngine::reset()
{
    tapeTrack.reset();
    printTape.reset();
    tapeState.reset();
    peakAtomic.store(0.0f);
}

void IntegratedAnalogPathEngine::setSettings(const AnalogPathSettings& settings)
{
    currentSettings = settings;
}

IntegratedAnalogPathTelemetry IntegratedAnalogPathEngine::getTelemetry() const
{
    IntegratedAnalogPathTelemetry t;
    t.peak = peakAtomic.load();
    t.tapeBiasStress = tapeState.tapeBiasStress;
    t.tapeCompression = tapeState.tapeCompression;
    t.lowFrequencySaturation = tapeState.lowFrequencySaturation;
    t.printTapeStress = tapeState.printTapeStress;
    return t;
}

void IntegratedAnalogPathEngine::processStereo(float* left, float* right, int numSamples)
{
    if (!left || !right || numSamples <= 0)
        return;

    const float inGain = tape::dbToGain(currentSettings.inputTrimDb);
    const float outGain = tape::dbToGain(currentSettings.outputTrimDb);

    for (int i = 0; i < numSamples; ++i)
    {
        left[i] *= inGain;
        right[i] *= inGain;
    }

    if (currentSettings.tapeTrackEnabled)
        tapeTrack.processStereo(left, right, numSamples, currentSettings.trackTapeProfile, tapeState);

    // Placeholder for existing ConsoleCore AnalogPhysicsCore / ProductionDspEngine.
    if (currentSettings.consoleEnabled)
    {
        const float consoleStress =
            std::clamp(0.18f * tapeState.tapeCompression +
                       0.12f * tapeState.lowFrequencySaturation +
                       0.08f * static_cast<float>(currentSettings.activeStemCount), 0.0f, 1.0f);

        const float gain = 1.0f / (1.0f + 0.25f * consoleStress);

        for (int i = 0; i < numSamples; ++i)
        {
            left[i] = tape::softClip(left[i] * gain, 0.12f + consoleStress * 0.35f);
            right[i] = tape::softClip(right[i] * gain, 0.12f + consoleStress * 0.35f);
        }
    }

    if (currentSettings.printTapeEnabled)
        printTape.processStereo(left, right, numSamples, currentSettings.printTapeProfile, tapeState);

    float peak = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        left[i] = tape::sanitize(left[i] * outGain);
        right[i] = tape::sanitize(right[i] * outGain);
        peak = std::max(peak, std::max(std::abs(left[i]), std::abs(right[i])));
    }

    peakAtomic.store(peak);
}
