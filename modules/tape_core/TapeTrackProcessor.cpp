#include "TapeTrackProcessor.h"
#include "TapeDspUtils.h"
#include <algorithm>
#include <cmath>

void TapeTrackProcessor::prepare(double sampleRate, int)
{
    fs = sampleRate > 0.0 ? sampleRate : 48000.0;
    reset();
}

void TapeTrackProcessor::reset()
{
    hysteresisL = hysteresisR = 0.0f;
    lowStateL = lowStateR = 0.0f;
    noiseState = 1;
}

void TapeTrackProcessor::processStereo(float* left, float* right, int numSamples,
                                       const TapeMachineProfile& p,
                                       TapeSharedState& state)
{
    if (!left || !right || numSamples <= 0)
        return;

    const float sat = tape::clamp01(p.saturationAmount);
    const float bias = tape::clamp01(p.biasAmount);
    const float hyst = tape::clamp01(p.hysteresisAmount);
    const float bump = tape::clamp01(p.headBumpAmount);
    const float hiss = tape::clamp01(p.hissAmount);
    const float repro = tape::clamp01(p.reproSoftness);
    const float trim = tape::dbToGain(p.outputTrimDb);

    const float lowCoeff = tape::onePoleCoeff(18.0f + 120.0f * (30.0f / std::max(1.0f, p.tapeSpeedIps)), fs);
    const float hystCoeff = tape::onePoleCoeff(45.0f + 320.0f * hyst, fs);

    float blockEnergy = 0.0f;
    float lowEnergy = 0.0f;
    float compressionAccum = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float l = tape::sanitize(left[i]);
        float r = tape::sanitize(right[i]);

        lowStateL = lowCoeff * lowStateL + (1.0f - lowCoeff) * l;
        lowStateR = lowCoeff * lowStateR + (1.0f - lowCoeff) * r;

        l += lowStateL * 0.22f * bump;
        r += lowStateR * 0.22f * bump;

        hysteresisL = hystCoeff * hysteresisL + (1.0f - hystCoeff) * l;
        hysteresisR = hystCoeff * hysteresisR + (1.0f - hystCoeff) * r;

        l += hysteresisL * 0.10f * hyst;
        r += hysteresisR * 0.10f * hyst;

        const float biasOffset = (bias - 0.5f) * 0.08f;
        const float drive = 0.10f + 0.70f * sat;

        const float preL = l;
        const float preR = r;

        l = tape::softClip(l + biasOffset, drive) - biasOffset * 0.65f;
        r = tape::softClip(r + biasOffset, drive) - biasOffset * 0.65f;

        l = (1.0f - 0.04f * repro) * l + 0.04f * repro * preL;
        r = (1.0f - 0.04f * repro) * r + 0.04f * repro * preR;

        const float n = tape::deterministicNoise(noiseState) * hiss * 0.0015f;
        l += n;
        r -= n * 0.7f;

        l *= trim;
        r *= trim;

        left[i] = tape::sanitize(l);
        right[i] = tape::sanitize(r);

        const float e = 0.5f * (l * l + r * r);
        blockEnergy += e;
        lowEnergy += 0.5f * (lowStateL * lowStateL + lowStateR * lowStateR);
        compressionAccum += std::abs(preL - l) + std::abs(preR - r);
    }

    const float inv = 1.0f / static_cast<float>(numSamples);
    state.tapeBiasStress = 0.98f * state.tapeBiasStress + 0.02f * std::min(1.0f, blockEnergy * inv * (0.8f + bias));
    state.lowFrequencySaturation = 0.98f * state.lowFrequencySaturation + 0.02f * std::min(1.0f, lowEnergy * inv * (1.0f + bump));
    state.tapeCompression = 0.98f * state.tapeCompression + 0.02f * std::min(1.0f, compressionAccum * inv * 4.0f);
    state.transportInstability = 0.98f * state.transportInstability + 0.02f * tape::clamp01(p.wowFlutterAmount);
    state.adjacentTrackCoupling = 0.98f * state.adjacentTrackCoupling + 0.02f * tape::clamp01(p.crosstalkAmount);
}
