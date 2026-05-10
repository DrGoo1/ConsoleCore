#pragma once

#include <juce_core/juce_core.h>

namespace consolecore
{
struct DevicePreset
{
    juce::String inputDeviceName;
    juce::String outputDeviceName;
    double sampleRate = 48000.0;
    int bufferSize = 512;
    int inputChannels = 16;
    int outputChannels = 2;

    juce::var toVar() const;
    static DevicePreset fromVar(const juce::var& value);

    bool saveToFile(const juce::File& file) const;
    static DevicePreset loadFromFile(const juce::File& file);
};
}
