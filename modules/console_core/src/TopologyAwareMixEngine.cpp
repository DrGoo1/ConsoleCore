#include "TopologyAwareMixEngine.h"
#include <algorithm>
#include <cmath>

namespace consolecore {

void TopologyAwareMixEngine::prepare(double sampleRate, int, int maxInputChannels) {
    sr = sampleRate > 1000.0 ? sampleRate : 48000.0;
    channelMemory.assign(static_cast<size_t>(std::max(2, maxInputChannels)), 0.0f);
}

void TopologyAwareMixEngine::setTopology(ConsoleTopology t) { topology = std::move(t); }
void TopologyAwareMixEngine::setProfile(ConsoleModelProfile p) { profile = std::move(p); }
void TopologyAwareMixEngine::setDrive(float v) { drive = std::clamp(v, 0.0f, 1.0f); }
void TopologyAwareMixEngine::setStress(float v) { stress = std::clamp(v, 0.0f, 1.0f); }
void TopologyAwareMixEngine::setCrosstalk(float v) { crosstalk = std::clamp(v, 0.0f, 1.0f); }
void TopologyAwareMixEngine::setTransformerWeight(float v) { transformerWeight = std::clamp(v, 0.0f, 1.0f); }
void TopologyAwareMixEngine::reset() { rail.reset(); std::fill(channelMemory.begin(), channelMemory.end(), 0.0f); lastActivity = 0.0f; }

float TopologyAwareMixEngine::dbToGain(float db) const noexcept { return std::pow(10.0f, db / 20.0f); }

float TopologyAwareMixEngine::processNonlinear(float x, float railMod, float channelBias) const noexcept {
    const float pregain = 1.0f + drive * 8.0f;
    const float asym = static_cast<float>(profile.transformerWeight) * 0.08f * drive;
    const float odd = (0.05f + static_cast<float>(profile.busSagDepth) * 0.8f) * 0.22f * drive;
    const float iron = transformerWeight * static_cast<float>(profile.transformerWeight) * 0.18f;
    float y = x * pregain * railMod;
    y += asym * (y * y) * (y >= 0.0f ? 1.0f : -1.0f);
    y += odd * y * y * y;
    y += channelBias * 0.0008f;
    y = std::tanh(y * (1.0f + iron)) / std::max(0.001f, pregain * 0.72f);
    return y;
}

void TopologyAwareMixEngine::process(const float* const* inputs, int numInputs, float* leftOut, float* rightOut, int numSamples) {
    if (!leftOut || !rightOut || numSamples <= 0) return;
    std::fill(leftOut, leftOut + numSamples, 0.0f);
    std::fill(rightOut, rightOut + numSamples, 0.0f);
    if (!inputs || numInputs <= 0) return;

    const bool anySolo = std::any_of(topology.stems.begin(), topology.stems.end(), [](const StemConfig& s){ return s.soloed; });
    float blockEnergy = 0.0f;
    int activeStems = 0;
    float sag = rail.getRailSag();
    const float recoveryCoeff = std::exp(-1.0f / static_cast<float>(std::max(1.0, sr * (profile.railRecoveryMs / 1000.0))));

    for (const auto& stem : topology.stems) {
        if (stem.muted || (anySolo && !stem.soloed)) continue;
        if (stem.inputLeft >= numInputs || stem.inputRight >= numInputs) continue;
        const float* inL = inputs[stem.inputLeft];
        const float* inR = inputs[stem.inputRight];
        if (!inL || !inR) continue;
        ++activeStems;
        const float gain = dbToGain(stem.trimDb);
        const float panL = std::sqrt(std::clamp(0.5f * (1.0f - stem.pan), 0.0f, 1.0f));
        const float panR = std::sqrt(std::clamp(0.5f * (1.0f + stem.pan), 0.0f, 1.0f));
        const float bias = (static_cast<float>((stem.stemId % 7) - 3) / 3.0f);

        for (int i = 0; i < numSamples; ++i) {
            const float absDemand = 0.5f * (std::abs(inL[i]) + std::abs(inR[i]));
            const float targetSag = absDemand * stress * static_cast<float>(profile.busSagDepth) * 4.0f;
            sag = sag * recoveryCoeff + targetSag * (1.0f - recoveryCoeff);
            const float railMod = 1.0f - std::clamp(sag * 0.45f, 0.0f, 0.6f);
            float l = processNonlinear(inL[i] * gain, railMod, bias);
            float r = processNonlinear(inR[i] * gain, railMod, -bias);
            const float xt = crosstalk * dbToGain(static_cast<float>(profile.crosstalkDb) + 48.0f) * (1.0f + sag * 2.0f);
            const float cl = l + r * xt;
            const float cr = r + l * xt;
            leftOut[i] += cl * panL;
            rightOut[i] += cr * panR;
            blockEnergy += absDemand;
        }
    }

    rail.setRailSag(sag);
    rail.setActiveChannels(activeStems);
    rail.advanceFrame();
    const float makeup = 1.0f / std::max(1.0f, std::sqrt(static_cast<float>(std::max(1, activeStems))));
    for (int i = 0; i < numSamples; ++i) { leftOut[i] *= makeup; rightOut[i] *= makeup; }
    lastActivity = blockEnergy / static_cast<float>(std::max(1, numSamples * std::max(1, activeStems)));
}

} // namespace consolecore
