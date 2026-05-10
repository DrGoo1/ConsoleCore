#include "FeatureExtractor.h"
#include <cmath>
#include <algorithm>

static float rms(const std::vector<float>& x)
{
    if (x.empty()) return 0.0f;
    double e = 0.0;
    for (float v : x) e += static_cast<double>(v) * v;
    return static_cast<float>(std::sqrt(e / x.size()));
}

static float peak(const std::vector<float>& x)
{
    float p = 0.0f;
    for (float v : x) p = std::max(p, std::abs(v));
    return p;
}

ExtractedFeatures FeatureExtractor::extractMonoPair(const std::vector<float>& dry, const std::vector<float>& wet, double)
{
    ExtractedFeatures f;
    f.dryRms = rms(dry);
    f.wetRms = rms(wet);
    f.dryPeak = peak(dry);
    f.wetPeak = peak(wet);

    f.crestFactor = f.wetRms > 1.0e-9f ? f.wetPeak / f.wetRms : 0.0f;
    f.gainDeltaDb = 20.0f * std::log10((f.wetRms + 1.0e-9f) / (f.dryRms + 1.0e-9f));

    // Scaffold estimates to be replaced by spectral bin/Goertzel analysis.
    f.estimatedThd = std::max(0.0f, (f.wetPeak - f.wetRms) * 0.01f);
    f.recoveryMs = 250.0f + 300.0f * std::clamp(f.estimatedThd, 0.0f, 1.0f);
    f.lowBandWeight = std::clamp(f.gainDeltaDb * -0.02f + 0.25f, 0.0f, 1.0f);
    f.highBandSoftening = std::clamp((f.crestFactor - 2.0f) * 0.08f, 0.0f, 1.0f);

    return f;
}