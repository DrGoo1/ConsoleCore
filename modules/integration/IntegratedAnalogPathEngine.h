#pragma once
#include "AnalogPathSettings.h"
#include "../tape_core/TapeTrackProcessor.h"
#include "../tape_core/TapePrintProcessor.h"
#include "../tape_core/TapeSharedState.h"
#include <atomic>

struct IntegratedAnalogPathTelemetry
{
    float peak = 0.0f;
    float tapeBiasStress = 0.0f;
    float tapeCompression = 0.0f;
    float lowFrequencySaturation = 0.0f;
    float printTapeStress = 0.0f;
};

class IntegratedAnalogPathEngine
{
public:
    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    void setSettings(const AnalogPathSettings& settings);
    const TapeSharedState& getTapeSharedState() const { return tapeState; }

    IntegratedAnalogPathTelemetry getTelemetry() const;
    void processStereo(float* left, float* right, int numSamples);

private:
    double fs = 48000.0;
    AnalogPathSettings currentSettings;

    TapeTrackProcessor tapeTrack;
    TapePrintProcessor printTape;
    TapeSharedState tapeState;

    std::atomic<float> peakAtomic {0.0f};
};
