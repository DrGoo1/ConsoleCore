#pragma once
#include "../analog_physics/AnalogPhysicsParameters.h"
#include <string>

struct ProfileValidationReport
{
    bool stable = false;
    float peakOutput = 0.0f;
    float railStress = 0.0f;
    float headroomGain = 1.0f;
    std::string message;
};

class ProfileValidator
{
public:
    static ProfileValidationReport validate(const AnalogPhysicsParameters& params);
};