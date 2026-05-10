#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "ConsoleCoreEngine.h"
#include "DevicePreset.h"
#include "LatencyProbe.h"
#include "LevelMeterState.h"
#include "RoutingMatrix.h"

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
    void openAudioSettings();
    void saveDevicePreset();
    void armLatencyProbe();
    void refreshStatusText();

    consolecore::ConsoleCoreEngine engine;
    consolecore::RoutingMatrix routing;
    consolecore::LevelMeterState meters;
    consolecore::LatencyProbe latencyProbe;

    juce::Label titleLabel;
    juce::Label statusLabel;
    juce::TextButton audioSettingsButton { "Audio Settings" };
    juce::TextButton savePresetButton { "Save Device Preset" };
    juce::TextButton latencyProbeButton { "Arm Latency Probe" };

    juce::AudioBuffer<float> scratchOutput;
    double lastSampleRate = 48000.0;
    int lastBlockSize = 512;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
