#include "DynamicCrosstalkMatrix.h"
#include "DspUtils.h"
#include <algorithm>
#include <cmath>

void DynamicCrosstalkMatrix::reset()
{
    smoothedAmount = 0.0f;
}

void DynamicCrosstalkMatrix::processStereo(float& left, float& right, float energy, float railStress, const AnalogPhysicsParameters& params)
{
    const float target = std::clamp(params.crosstalkAmount * (0.015f + 0.08f * energy + 0.04f * railStress), 0.0f, 0.075f);
    smoothedAmount = 0.995f * smoothedAmount + 0.005f * target;

    const float l = left;
    const float r = right;

    left  = l + r * smoothedAmount;
    right = r + l * smoothedAmount;
}