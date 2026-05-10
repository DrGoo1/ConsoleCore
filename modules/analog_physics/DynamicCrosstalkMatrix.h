#pragma once
#include "AnalogPhysicsParameters.h"

class DynamicCrosstalkMatrix
{
public:
    void reset();
    void processStereo(float& left, float& right, float energy, float railStress, const AnalogPhysicsParameters& params);

private:
    float smoothedAmount = 0.0f;
};