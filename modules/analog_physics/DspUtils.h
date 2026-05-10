#pragma once
#include <cmath>
#include <algorithm>
#include <limits>

namespace cc
{
    inline float clamp01(float v) { return std::clamp(v, 0.0f, 1.0f); }
    inline float dbToGain(float db) { return std::pow(10.0f, db / 20.0f); }
    inline float softClip(float x, float drive)
    {
        const float g = 1.0f + 8.0f * clamp01(drive);
        return std::tanh(x * g) / std::tanh(g);
    }
    inline bool isFinite(float x) { return std::isfinite(x); }
    inline float sanitize(float x)
    {
        return std::isfinite(x) ? x : 0.0f;
    }
    inline float onePoleCoeff(float ms, double sampleRate)
    {
        const float t = std::max(0.001f, ms) * 0.001f;
        return std::exp(-1.0f / static_cast<float>(t * sampleRate));
    }
}