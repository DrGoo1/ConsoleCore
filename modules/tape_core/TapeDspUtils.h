#pragma once
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace tape
{
    inline float clamp01(float v) { return std::clamp(v, 0.0f, 1.0f); }
    inline float dbToGain(float db) { return std::pow(10.0f, db / 20.0f); }
    inline float sanitize(float x) { return std::isfinite(x) ? x : 0.0f; }

    inline float softClip(float x, float amount)
    {
        const float a = 1.0f + 8.0f * clamp01(amount);
        return std::tanh(x * a) / std::tanh(a);
    }

    inline float onePoleCoeff(float ms, double sampleRate)
    {
        const float t = std::max(0.001f, ms) * 0.001f;
        return std::exp(-1.0f / static_cast<float>(t * sampleRate));
    }

    inline float deterministicNoise(std::uint32_t& state)
    {
        state = 1664525u * state + 1013904223u;
        const float u = static_cast<float>((state >> 8) & 0xFFFFFF) / static_cast<float>(0xFFFFFF);
        return (2.0f * u - 1.0f);
    }
}
