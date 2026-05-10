#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

namespace consolecore
{
struct RoutingPair
{
    int inputChannel = 0;
    int outputChannel = 0;
    float gain = 1.0f;
};

class RoutingMatrix
{
public:
    void clear();
    void addRoute(int inputChannel, int outputChannel, float gain = 1.0f);
    void setIdentity(int channelCount);
    void setStereoFoldDown(int inputChannelCount);

    void process(const juce::AudioBuffer<float>& input,
                 juce::AudioBuffer<float>& output,
                 int numSamples) const noexcept;

    const std::vector<RoutingPair>& getRoutes() const noexcept { return routes; }

private:
    std::vector<RoutingPair> routes;
};
}
