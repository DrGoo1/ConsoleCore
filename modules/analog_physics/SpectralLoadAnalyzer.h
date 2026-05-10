#pragma once

struct SpectralLoad
{
    float low = 0.0f;
    float mid = 0.0f;
    float high = 0.0f;
    float broadband = 0.0f;
};

class SpectralLoadAnalyzer
{
public:
    void prepare(double sampleRate);
    void reset();
    SpectralLoad analyzeBlock(const float* left, const float* right, int numSamples);

private:
    double fs = 48000.0;
    float lowStateL = 0.0f;
    float lowStateR = 0.0f;
    float prevL = 0.0f;
    float prevR = 0.0f;
};