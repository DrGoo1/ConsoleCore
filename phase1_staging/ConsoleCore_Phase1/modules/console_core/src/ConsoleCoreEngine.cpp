#include "ConsoleCoreEngine.h"

namespace consolecore
{
void ConsoleCoreEngine::prepare(double sampleRate, int maximumBlockSize, int channelCount)
{
    currentSampleRate = sampleRate;
    currentMaxBlockSize = maximumBlockSize;
    currentChannelCount = channelCount;
    reset();
}

void ConsoleCoreEngine::reset()
{
    railState.reset();
}

void ConsoleCoreEngine::process(juce::AudioBuffer<float>& buffer) noexcept
{
    // Phase 0 intentional pass-through.
    // Phase 2 will add topology-aware rail, crosstalk, nonlinear headroom, and memory behavior.
    int active = 0;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getReadPointer(ch);
        for (int n = 0; n < buffer.getNumSamples(); ++n)
        {
            if (std::abs(data[n]) > 1.0e-5f)
            {
                ++active;
                break;
            }
        }
    }

    railState.setActiveChannels(active);
    railState.setRailSag(0.0f);
    railState.advanceFrame();
}
}
