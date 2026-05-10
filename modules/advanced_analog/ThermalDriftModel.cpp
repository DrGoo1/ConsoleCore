#include "ThermalDriftModel.h"
#include <algorithm>

void ThermalDriftModel::reset()
{
    temperature = 0.0f;
}

float ThermalDriftModel::process(float inputEnergy, float)
{
    temperature = 0.9995f * temperature + 0.0005f * inputEnergy;
    return std::clamp(temperature, 0.0f, 1.0f);
}