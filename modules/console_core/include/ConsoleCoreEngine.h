#pragma once

#include "ConsoleModelProfile.h"
#include "SharedRailState.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace consolecore
{
class ConsoleCoreEngine
{
public:
    void prepare(double sampleRate, int maximumBlockSize, int channelCount);
    void reset();
    void process(juce::AudioBuffer<float>& buffer) noexcept;

    const SharedRailState& getRailState() const noexcept { return railState; }
    void setProfile(ConsoleModelProfile nextProfile) { profile = std::move(nextProfile); }
    const ConsoleModelProfile& getProfile() const noexcept { return profile; }

private:
    double currentSampleRate { 48000.0 };
    int currentMaxBlockSize { 512 };
    int currentChannelCount { 2 };
    ConsoleModelProfile profile;
    SharedRailState railState;
};
}
