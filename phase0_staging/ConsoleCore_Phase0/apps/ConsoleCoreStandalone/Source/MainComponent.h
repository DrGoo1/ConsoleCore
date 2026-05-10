#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "ConsoleCoreEngine.h"
#include "LevelMeterState.h"

class MainComponent final : public juce::AudioAppComponent, private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void computeMeters(const juce::AudioBuffer<float>& buffer);

    consolecore::ConsoleCoreEngine engine;
    consolecore::LevelMeterState meters;

    juce::Label titleLabel;
    juce::Label statusLabel;
    juce::TextButton audioSettingsButton { "Audio Settings" };
    juce::TextButton savePresetButton { "Save Device Preset" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
