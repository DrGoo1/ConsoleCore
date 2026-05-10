#pragma once
#include "ConsoleModelProfile.h"
#include "SharedRailState.h"
#include "TopologyTypes.h"
#include <vector>

namespace consolecore {

class TopologyAwareMixEngine {
public:
    void prepare(double sampleRate, int maxBlockSize, int maxInputChannels);
    void setTopology(ConsoleTopology topology);
    void setProfile(ConsoleModelProfile profile);
    void setDrive(float value01);
    void setStress(float value01);
    void setCrosstalk(float value01);
    void setTransformerWeight(float value01);
    void reset();

    void process(const float* const* inputs, int numInputs, float* leftOut, float* rightOut, int numSamples);

    float getRailSag() const noexcept { return rail.getRailSag(); }
    float getActivity() const noexcept { return lastActivity; }

private:
    float processNonlinear(float x, float railMod, float channelBias) const noexcept;
    float dbToGain(float db) const noexcept;

    double sr = 48000.0;
    ConsoleTopology topology;
    ConsoleModelProfile profile;
    SharedRailState rail;
    float drive = 0.15f;
    float stress = 0.15f;
    float crosstalk = 0.05f;
    float transformerWeight = 0.1f;
    float lastActivity = 0.0f;
    std::vector<float> channelMemory;
};

} // namespace consolecore
