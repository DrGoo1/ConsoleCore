#pragma once

class ThermalDriftModel
{
public:
    void reset();
    float process(float inputEnergy, float secondsElapsed);

private:
    float temperature = 0.0f;
};