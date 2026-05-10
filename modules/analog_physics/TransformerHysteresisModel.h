#pragma once
#include "AnalogPhysicsParameters.h"

class TransformerHysteresisModel
{
public:
    void prepare(double sampleRate);
    void reset();

    float processSample(float x, float railStress, const AnalogPhysicsParameters& params);

private:
    double fs = 48000.0;
    float magneticMemory = 0.0f;
};