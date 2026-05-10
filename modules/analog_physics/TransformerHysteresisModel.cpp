#include "TransformerHysteresisModel.h"
#include "DspUtils.h"
#include <algorithm>
#include <cmath>

void TransformerHysteresisModel::prepare(double sampleRate)
{
    fs = sampleRate > 0.0 ? sampleRate : 48000.0;
    reset();
}

void TransformerHysteresisModel::reset()
{
    magneticMemory = 0.0f;
}

float TransformerHysteresisModel::processSample(float x, float railStress, const AnalogPhysicsParameters& params)
{
    const float w = cc::clamp01(params.transformerWeight);
    const float memoryCoeff = cc::onePoleCoeff(35.0f + 250.0f * w, fs);

    magneticMemory = memoryCoeff * magneticMemory + (1.0f - memoryCoeff) * x;

    const float asym = 0.08f * w * (1.0f + railStress);
    const float biased = x + magneticMemory * (0.12f * w) + asym;
    const float driven = cc::softClip(biased, params.drive + 0.4f * w + 0.3f * railStress);

    return driven - asym * 0.65f;
}