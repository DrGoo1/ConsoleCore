#pragma once
#include <string>

enum class MeasurementType
{
    Sweep,
    Impulse,
    Multitone,
    NoiseFloor,
    Crosstalk,
    DynamicStress
};

struct MeasurementPass
{
    MeasurementType type = MeasurementType::Sweep;
    float levelDbfs = -18.0f;
    std::string notes;
};