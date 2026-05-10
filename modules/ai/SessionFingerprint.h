#pragma once
#include "FeatureVector.h"
#include <vector>

struct SessionFingerprint
{
    FeatureVector aggregate;
    int activeStemCount = 0;
    float averageRailStress = 0.0f;
    float dynamicRangeEstimate = 0.0f;
};