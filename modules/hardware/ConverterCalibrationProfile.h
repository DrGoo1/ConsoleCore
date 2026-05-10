#pragma once
#include <string>

struct ConverterCalibrationProfile
{
    std::string converterName;
    int sampleRate = 48000;
    float dbfsReference = -18.0f;
    float measuredNoiseFloorDb = -110.0f;
    float measuredDynamicRangeDb = 118.0f;
};