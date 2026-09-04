#include "RailPhysicsEngine.h"
#include "DspUtils.h"
#include <algorithm>
#include <cmath>

void RailPhysicsEngine::prepare(double sampleRate)
{
    fs = sampleRate > 0.0 ? sampleRate : 48000.0;
    reset();
}

void RailPhysicsEngine::reset()
{
    railVoltage = 1.0f;
    railStress = 0.0f;
    recoveryState = 0.0f;
}

void RailPhysicsEngine::update(float blockEnergy, int activeChannels, const AnalogPhysicsParameters& params, float distributionStress, int samplesElapsed)
{
    const float channelFactor = std::sqrt(std::max(1, activeChannels)) * std::max(0.0f, params.channelCountRailLoadFactor);
    const float distributionFactor = 1.0f + std::clamp(distributionStress, 0.0f, 1.0f) * std::clamp(params.channelDistributionSensitivity, 0.0f, 1.0f);
    const float targetStress = cc::clamp01(blockEnergy * (1.0f + channelFactor) * distributionFactor * (0.5f + 2.5f * params.railStressAmount));

    const float sampleCoeff = cc::onePoleCoeff(params.railRecoveryMs, fs);
    const float coeff = std::pow(sampleCoeff, static_cast<float>(std::max(1, samplesElapsed)));
    railStress = coeff * railStress + (1.0f - coeff) * targetStress;

    recoveryState = coeff * recoveryState + (1.0f - coeff) * (1.0f - railStress);
    railVoltage = std::clamp(1.0f - std::max(0.0f, params.railVoltageDroopFactor) * railStress, 0.55f, 1.05f);
}