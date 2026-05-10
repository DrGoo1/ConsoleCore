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

void RailPhysicsEngine::update(float blockEnergy, int activeChannels, const AnalogPhysicsParameters& params)
{
    const float channelFactor = std::sqrt(std::max(1, activeChannels)) * 0.18f;
    const float targetStress = cc::clamp01(blockEnergy * (1.0f + channelFactor) * (0.5f + 2.5f * params.railStressAmount));

    const float coeff = cc::onePoleCoeff(params.railRecoveryMs, fs);
    railStress = coeff * railStress + (1.0f - coeff) * targetStress;

    recoveryState = coeff * recoveryState + (1.0f - coeff) * (1.0f - railStress);
    railVoltage = std::clamp(1.0f - 0.35f * railStress, 0.55f, 1.05f);
}