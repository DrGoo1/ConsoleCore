#include "ParameterInterpolator.h"

float ParameterInterpolator::lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}