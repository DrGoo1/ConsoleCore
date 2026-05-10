#pragma once
#include "AnalogPhysicsParameters.h"

class RailPhysicsEngine
{
public:
    void prepare(double sampleRate);
    void reset();

    void update(float blockEnergy, int activeChannels, const AnalogPhysicsParameters& params);

    float getRailVoltage() const { return railVoltage; }
    float getRailStress() const { return railStress; }
    float getRecoveryState() const { return recoveryState; }

private:
    double fs = 48000.0;
    float railVoltage = 1.0f;
    float railStress = 0.0f;
    float recoveryState = 0.0f;
};