#pragma once
#include "ExtractedFeatures.h"
#include "../analog_physics/AnalogPhysicsParameters.h"

class ModelFitter
{
public:
    static AnalogPhysicsParameters fitBasicProfile(const ExtractedFeatures& features);
};