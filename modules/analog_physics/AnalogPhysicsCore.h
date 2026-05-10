#pragma once
#include "AnalogPhysicsParameters.h"
#include "RailPhysicsEngine.h"
#include "TemporalMemoryEngine.h"
#include "TransformerHysteresisModel.h"
#include "DynamicCrosstalkMatrix.h"
#include "SpectralLoadAnalyzer.h"
#include "AdaptiveHeadroomModel.h"

struct AnalogPhysicsTelemetry
{
    float railVoltage = 1.0f;
    float railStress = 0.0f;
    float memory = 0.0f;
    float headroomGain = 1.0f;
    float peakOutput = 0.0f;
    SpectralLoad spectralLoad;
};

class AnalogPhysicsCore
{
public:
    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    void setParameters(const AnalogPhysicsParameters& p);
    const AnalogPhysicsParameters& getParameters() const { return params; }

    void processBlock(float* left, float* right, int numSamples, int activeChannels);

    const AnalogPhysicsTelemetry& getTelemetry() const { return telemetry; }

private:
    AnalogPhysicsParameters params;
    RailPhysicsEngine rail;
    TemporalMemoryEngine temporalMemory;
    TransformerHysteresisModel transformerL;
    TransformerHysteresisModel transformerR;
    DynamicCrosstalkMatrix crosstalk;
    SpectralLoadAnalyzer spectral;
    AdaptiveHeadroomModel headroom;
    AnalogPhysicsTelemetry telemetry;
    double fs = 48000.0;
};