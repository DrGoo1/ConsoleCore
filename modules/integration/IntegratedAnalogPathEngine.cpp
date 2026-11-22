#include "IntegratedAnalogPathEngine.h"
#include "../tape_core/TapeDspUtils.h"
#include <algorithm>
#include <cmath>

void IntegratedAnalogPathEngine::prepare(double sampleRate, int maxBlockSize)
{
    fs = sampleRate > 0.0 ? sampleRate : 48000.0;
    maxBlock = maxBlockSize > 0 ? maxBlockSize : 512;
    prepared = true;
    tapeTrack.prepare(fs, maxBlockSize);
    if (trackMachineSelected)
        trackMachineSelected = trackMachine.prepare(fs, maxBlock, 2, trackMachineSpec, trackMachineRun);
    if (consoleStageSelected)
        consoleStageSelected = consoleStage.prepare(fs, 2, consoleStageSpec, consoleStageRun);
    printTape.prepare(fs, maxBlockSize);
    reset();
}

void IntegratedAnalogPathEngine::reset()
{
    tapeTrack.reset();
    trackMachine.reset();
    consoleStage.reset();
    printTape.reset();
    tapeState.reset();
    peakAtomic.store(0.0f);
}

void IntegratedAnalogPathEngine::setSettings(const AnalogPathSettings& settings)
{
    currentSettings = settings;
}

bool IntegratedAnalogPathEngine::setTrackMachine(const tape::MachineSpec& spec, const tape::RunSettings& run)
{
    if (spec.findSpeed(run.ips) == nullptr)
        return false;
    trackMachineSpec = spec;
    trackMachineRun = run;
    trackMachineSelected = true;
    if (prepared)
        trackMachineSelected = trackMachine.prepare(fs, maxBlock, 2, trackMachineSpec, trackMachineRun);
    return trackMachineSelected;
}

void IntegratedAnalogPathEngine::clearTrackMachine()
{
    trackMachineSelected = false;
}

bool IntegratedAnalogPathEngine::setConsoleStage(const console::StageSpec& spec, const console::StageRunSettings& run)
{
    consoleStageSpec = spec;
    consoleStageRun = run;
    consoleStageSelected = true;
    if (prepared)
        consoleStageSelected = consoleStage.prepare(fs, 2, consoleStageSpec, consoleStageRun);
    return consoleStageSelected;
}

void IntegratedAnalogPathEngine::clearConsoleStage()
{
    consoleStageSelected = false;
}

int IntegratedAnalogPathEngine::latencySamples() const
{
    int total = 0;
    if (currentSettings.tapeTrackEnabled && hasTrackMachine())
        total += trackMachine.latencySamples();
    if (currentSettings.consoleEnabled && hasConsoleStage())
        total += console::PhysicalConsoleStage::latencySamples();
    return total;
}

IntegratedAnalogPathTelemetry IntegratedAnalogPathEngine::getTelemetry() const
{
    IntegratedAnalogPathTelemetry t;
    t.peak = peakAtomic.load();
    t.tapeBiasStress = tapeState.tapeBiasStress;
    t.tapeCompression = tapeState.tapeCompression;
    t.lowFrequencySaturation = tapeState.lowFrequencySaturation;
    t.printTapeStress = tapeState.printTapeStress;
    t.physicalTrackMachineActive = currentSettings.tapeTrackEnabled && hasTrackMachine();
    t.physicalConsoleStageActive = currentSettings.consoleEnabled && hasConsoleStage();
    t.latencySamples = latencySamples();
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

    if (currentSettings.tapeTrackEnabled && hasTrackMachine())
    {
        double before = 0.0, after = 0.0;
        for (int i = 0; i < numSamples; ++i)
            before += static_cast<double>(left[i]) * left[i] + static_cast<double>(right[i]) * right[i];

        float* channels[2] = { left, right };
        trackMachine.process(channels, 2, numSamples);

        for (int i = 0; i < numSamples; ++i)
            after += static_cast<double>(left[i]) * left[i] + static_cast<double>(right[i]) * right[i];

        // Level lost in the machine this block, as a rough 0..1 figure for the console stage and meters.
        const float lost = before > 1.0e-12 ? std::clamp(1.0f - static_cast<float>(std::sqrt(after / before)), 0.0f, 1.0f) : 0.0f;
        tapeState.tapeCompression = 0.98f * tapeState.tapeCompression + 0.02f * std::min(1.0f, lost * 4.0f);
    }
    else if (currentSettings.tapeTrackEnabled)
        tapeTrack.processStereo(left, right, numSamples, currentSettings.trackTapeProfile, tapeState);

    if (currentSettings.consoleEnabled && hasConsoleStage())
    {
        float* channels[2] = { left, right };
        consoleStage.process(channels, 2, numSamples);
    }
    // Placeholder for existing ConsoleCore AnalogPhysicsCore / ProductionDspEngine.
    else if (currentSettings.consoleEnabled)
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
