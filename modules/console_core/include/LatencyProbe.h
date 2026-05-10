#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace consolecore
{
class LatencyProbe
{
public:
    void prepare(double sampleRate);
    void reset();
    void arm();
    bool isArmed() const noexcept { return armed; }
    int getDetectedLatencySamples() const noexcept { return detectedLatencySamples; }

    void injectPulse(juce::AudioBuffer<float>& buffer, int channel) noexcept;
    void scanReturn(const juce::AudioBuffer<float>& buffer, int channel) noexcept;

private:
    double sr = 48000.0;
    bool armed = false;
    bool pulseWritten = false;
    int samplesSincePulse = 0;
    int detectedLatencySamples = -1;
};
}
