#pragma once
#include <vector>
#include <atomic>

struct ProductionDspSettings
{
    bool bypass = false;
    bool deltaMonitor = false;
    float inputTrimDb = 0.0f;
    float outputTrimDb = 0.0f;
    int activeStemCount = 2;
};

struct ProductionDspTelemetry
{
    float peak = 0.0f;
    float railStress = 0.0f;
    float latencySamples = 0.0f;
};

class ProductionDspEngine
{
public:
    void prepare(double sampleRate, int maxBlockSize);
    void reset();
    void setSettings(const ProductionDspSettings& settings);
    ProductionDspTelemetry getTelemetry() const;

    void processStereo(float* left, float* right, int numSamples);

private:
    double fs = 48000.0;
    ProductionDspSettings currentSettings;
    std::atomic<float> peakAtomic {0.0f};
    std::atomic<float> railStressAtomic {0.0f};
};