#pragma once
#include <string>

struct TapeMachineProfile
{
    std::string name = "Generic 2-inch 24-track";
    float tapeSpeedIps = 15.0f;
    float trackWidth = 1.0f;
    float biasAmount = 0.50f;
    float saturationAmount = 0.35f;
    float headBumpAmount = 0.25f;
    float wowFlutterAmount = 0.05f;
    float hissAmount = 0.02f;
    float hysteresisAmount = 0.25f;
    float reproSoftness = 0.18f;
    float crosstalkAmount = 0.04f;
    float outputTrimDb = 0.0f;
};
