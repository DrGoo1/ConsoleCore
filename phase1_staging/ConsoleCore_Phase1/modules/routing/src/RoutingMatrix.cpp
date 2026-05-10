#include "RoutingMatrix.h"

namespace consolecore
{
void RoutingMatrix::clear()
{
    routes.clear();
}

void RoutingMatrix::addRoute(int inputChannel, int outputChannel, float gain)
{
    routes.push_back({ inputChannel, outputChannel, gain });
}

void RoutingMatrix::setIdentity(int channelCount)
{
    clear();
    for (int ch = 0; ch < channelCount; ++ch)
        addRoute(ch, ch, 1.0f);
}

void RoutingMatrix::setStereoFoldDown(int inputChannelCount)
{
    clear();
    for (int ch = 0; ch < inputChannelCount; ++ch)
        addRoute(ch, ch % 2, 1.0f);
}

void RoutingMatrix::process(const juce::AudioBuffer<float>& input,
                            juce::AudioBuffer<float>& output,
                            int numSamples) const noexcept
{
    output.clear();

    for (const auto& route : routes)
    {
        if (route.inputChannel < 0 || route.outputChannel < 0)
            continue;
        if (route.inputChannel >= input.getNumChannels() || route.outputChannel >= output.getNumChannels())
            continue;

        output.addFrom(route.outputChannel, 0, input, route.inputChannel, 0, numSamples, route.gain);
    }
}
}
