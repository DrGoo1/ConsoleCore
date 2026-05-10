#include "DynamicImpedanceModel.h"
#include <algorithm>

float DynamicImpedanceModel::computeGain(float lowEnergy, float railStress) const
{
    return std::clamp(1.0f / (1.0f + 0.25f * lowEnergy + 0.35f * railStress), 0.55f, 1.05f);
}