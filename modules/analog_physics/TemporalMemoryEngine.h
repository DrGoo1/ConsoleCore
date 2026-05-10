#pragma once

class TemporalMemoryEngine
{
public:
    void prepare(double sampleRate);
    void reset();

    float processEnergy(float instantaneousEnergy, float attackMs, float releaseMs);
    float getMemory() const { return memory; }

private:
    double fs = 48000.0;
    float memory = 0.0f;
};