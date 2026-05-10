#pragma once
#include <vector>
#include <string>

struct FeatureVector
{
    std::string sourceName;

    float rms = 0.0f;
    float peak = 0.0f;
    float crestFactor = 0.0f;

    float lowEnergy = 0.0f;
    float midEnergy = 0.0f;
    float highEnergy = 0.0f;

    float harmonicDensity = 0.0f;
    float transientSharpness = 0.0f;
    float stereoCorrelation = 0.0f;

    float railStressMean = 0.0f;
    float railStressVariance = 0.0f;

    std::vector<float> embedding;
};