#pragma once

class DynamicImpedanceModel
{
public:
    float computeGain(float lowEnergy, float railStress) const;
};