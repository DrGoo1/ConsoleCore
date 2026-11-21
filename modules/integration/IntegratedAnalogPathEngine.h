#pragma once
#include "AnalogPathSettings.h"
#include "../tape_core/TapeTrackProcessor.h"
#include "../tape_core/TapePrintProcessor.h"
#include "../tape_core/TapeSharedState.h"
#include "../tape_core/PhysicalTapeMachine.h"
#include <atomic>

struct IntegratedAnalogPathTelemetry
{
    float peak = 0.0f;
    float tapeBiasStress = 0.0f;
    float tapeCompression = 0.0f;
    float lowFrequencySaturation = 0.0f;
    float printTapeStress = 0.0f;
    bool physicalTrackMachineActive = false;
    int latencySamples = 0;
};

class IntegratedAnalogPathEngine
{
public:
    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    void setSettings(const AnalogPathSettings& settings);
    const TapeSharedState& getTapeSharedState() const { return tapeState; }

    // Selects a documentation-derived machine profile for the track tape stage. While one
    // is selected, the track tape stage runs tape::PhysicalTapeMachine instead of the
    // generic TapeTrackProcessor. Allocates and calibrates, so call it from a non-audio
    // thread while audio is stopped. Returns false (and keeps the generic processor) if
    // the profile has no entry for the requested speed.
    bool setTrackMachine(const tape::MachineSpec& spec, const tape::RunSettings& run);
    void clearTrackMachine();
    bool hasTrackMachine() const { return trackMachineSelected && trackMachine.isReady(); }

    // Delay the path adds, in samples, with the current settings. Report it to the host.
    int latencySamples() const;

    IntegratedAnalogPathTelemetry getTelemetry() const;
    void processStereo(float* left, float* right, int numSamples);

private:
    double fs = 48000.0;
    AnalogPathSettings currentSettings;

    int maxBlock = 512;
    bool prepared = false;
    bool trackMachineSelected = false;
    tape::MachineSpec trackMachineSpec;
    tape::RunSettings trackMachineRun;
    tape::PhysicalTapeMachine trackMachine;

    TapeTrackProcessor tapeTrack;
    TapePrintProcessor printTape;
    TapeSharedState tapeState;

    std::atomic<float> peakAtomic {0.0f};
};
