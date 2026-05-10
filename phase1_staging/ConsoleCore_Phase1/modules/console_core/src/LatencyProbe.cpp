#include "LatencyProbe.h"
#include <cmath>

namespace consolecore
{
void LatencyProbe::prepare(double sampleRate)
{
    sr = sampleRate;
    reset();
}

void LatencyProbe::reset()
{
    armed = false;
    pulseWritten = false;
    samplesSincePulse = 0;
    detectedLatencySamples = -1;
}

void LatencyProbe::arm()
{
    armed = true;
    pulseWritten = false;
    samplesSincePulse = 0;
    detectedLatencySamples = -1;
}

void LatencyProbe::injectPulse(juce::AudioBuffer<float>& buffer, int channel) noexcept
{
    if (!armed || pulseWritten || channel < 0 || channel >= buffer.getNumChannels())
        return;

    if (buffer.getNumSamples() > 0)
    {
        buffer.setSample(channel, 0, 0.9f);
        pulseWritten = true;
    }
}

void LatencyProbe::scanReturn(const juce::AudioBuffer<float>& buffer, int channel) noexcept
{
    if (!armed || !pulseWritten || detectedLatencySamples >= 0 || channel < 0 || channel >= buffer.getNumChannels())
        return;

    const auto* data = buffer.getReadPointer(channel);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        if (std::abs(data[i]) > 0.5f)
        {
            detectedLatencySamples = samplesSincePulse + i;
            armed = false;
            return;
        }
    }

    samplesSincePulse += buffer.getNumSamples();

    if (samplesSincePulse > static_cast<int>(sr * 5.0))
        armed = false;
}
}
