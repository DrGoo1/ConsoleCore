#include "SpectralLoadAnalyzer.h"
#include "DspUtils.h"
#include <cmath>
#include <algorithm>

void SpectralLoadAnalyzer::prepare(double sampleRate)
{
    fs = sampleRate > 0.0 ? sampleRate : 48000.0;
    reset();
}

void SpectralLoadAnalyzer::reset()
{
    lowStateL = lowStateR = prevL = prevR = 0.0f;
}

SpectralLoad SpectralLoadAnalyzer::analyzeBlock(const float* left, const float* right, int numSamples)
{
    SpectralLoad load;
    if (numSamples <= 0) return load;

    // Simple non-FFT energy bands:
    // low = slow one-pole state, high = first difference, mid = residual approximation.
    const float a = 0.995f;

    double lowE = 0.0, midE = 0.0, highE = 0.0, broadE = 0.0;

    for (int i = 0; i < numSamples; ++i)
    {
        const float l = left ? left[i] : 0.0f;
        const float r = right ? right[i] : l;
        const float mono = 0.5f * (l + r);

        lowStateL = a * lowStateL + (1.0f - a) * l;
        lowStateR = a * lowStateR + (1.0f - a) * r;
        const float low = 0.5f * (lowStateL + lowStateR);

        const float high = mono - 0.5f * (prevL + prevR);
        const float mid = mono - low - 0.25f * high;

        prevL = l;
        prevR = r;

        lowE += low * low;
        midE += mid * mid;
        highE += high * high;
        broadE += mono * mono;
    }

    const float inv = 1.0f / static_cast<float>(numSamples);
    load.low = std::sqrt(static_cast<float>(lowE) * inv);
    load.mid = std::sqrt(static_cast<float>(midE) * inv);
    load.high = std::sqrt(static_cast<float>(highE) * inv);
    load.broadband = std::sqrt(static_cast<float>(broadE) * inv);
    return load;
}