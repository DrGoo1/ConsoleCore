#include "DevicePreset.h"

namespace consolecore
{
juce::var DevicePreset::toVar() const
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("schema", "com.consolecore.devicePreset.v1");
    obj->setProperty("inputDeviceName", inputDeviceName);
    obj->setProperty("outputDeviceName", outputDeviceName);
    obj->setProperty("sampleRate", sampleRate);
    obj->setProperty("bufferSize", bufferSize);
    obj->setProperty("inputChannels", inputChannels);
    obj->setProperty("outputChannels", outputChannels);
    return obj;
}

DevicePreset DevicePreset::fromVar(const juce::var& value)
{
    DevicePreset preset;
    if (auto* obj = value.getDynamicObject())
    {
        preset.inputDeviceName = obj->getProperty("inputDeviceName").toString();
        preset.outputDeviceName = obj->getProperty("outputDeviceName").toString();
        preset.sampleRate = static_cast<double>(obj->getProperty("sampleRate"));
        preset.bufferSize = static_cast<int>(obj->getProperty("bufferSize"));
        preset.inputChannels = static_cast<int>(obj->getProperty("inputChannels"));
        preset.outputChannels = static_cast<int>(obj->getProperty("outputChannels"));
    }
    return preset;
}

bool DevicePreset::saveToFile(const juce::File& file) const
{
    return file.replaceWithText(juce::JSON::toString(toVar(), true));
}

DevicePreset DevicePreset::loadFromFile(const juce::File& file)
{
    auto parsed = juce::JSON::parse(file);
    return fromVar(parsed);
}
}
