#pragma once
#include "ExtractedFeatures.h"
#include <vector>

class FeatureExtractor
{
public:
    static ExtractedFeatures extractMonoPair(const std::vector<float>& dry, const std::vector<float>& wet, double sampleRate);
};