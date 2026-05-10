#pragma once
#include "AnalogPhysicsParameters.h"
#include "SpectralLoadAnalyzer.h"

class AdaptiveHeadroomModel
{
public:
    float computeGain(float railStress, const SpectralLoad& load, int activeChannels, const AnalogPhysicsParameters& params) const;
};