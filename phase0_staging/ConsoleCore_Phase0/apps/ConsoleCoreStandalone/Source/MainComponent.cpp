#include "MainComponent.h"

MainComponent::MainComponent()
{
    setOpaque(true);

    titleLabel.setText("ConsoleCore Phase 0", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::FontOptions(28.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    statusLabel.setText("Pass-through engine ready. Configure Loopback/CoreAudio device.", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(statusLabel);

    audioSettingsButton.onClick = [this]
    {
        auto selector = std::make_unique<juce::AudioDeviceSelectorComponent>(deviceManager,
                                                                            0, 64,
                                                                            0, 64,
                                                                            true, true, true, false);
        selector->setSize(720, 520);
        juce::DialogWindow::LaunchOptions opts;
        opts.content.setOwned(selector.release());
        opts.dialogTitle = "ConsoleCore Audio Settings";
        opts.dialogBackgroundColour = juce::Colours::darkgrey;
        opts.escapeKeyTriggersCloseButton = true;
        opts.useNativeTitleBar = true;
        opts.resizable = true;
        opts.launchAsync();
    };
    addAndMakeVisible(audioSettingsButton);
    addAndMakeVisible(savePresetButton);

    setAudioChannels(64, 64);
    meters.resize(64);
    startTimerHz(24);
    setSize(980, 640);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    engine.prepare(sampleRate, samplesPerBlockExpected, 64);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* buffer = bufferToFill.buffer;
    if (buffer == nullptr) return;

    // Phase 0: leave host/device input in place and run pass-through telemetry.
    engine.process(*buffer);
    computeMeters(*buffer);
}

void MainComponent::releaseResources()
{
    engine.reset();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff111118));
    g.setColour(juce::Colour(0xff9f7aea));
    g.drawRoundedRectangle(getLocalBounds().reduced(16).toFloat(), 16.0f, 2.0f);

    const auto meterArea = getLocalBounds().withTrimmedTop(150).reduced(28);
    const int channelsToShow = juce::jmin(16, meters.getNumChannels());
    const int rowH = 22;

    g.setColour(juce::Colours::white.withAlpha(0.85f));
    g.setFont(14.0f);
    g.drawText("Input/Output meter telemetry placeholder", meterArea.removeFromTop(28), juce::Justification::centredLeft);

    for (int ch = 0; ch < channelsToShow; ++ch)
    {
        const auto row = juce::Rectangle<int>(32, 185 + ch * rowH, getWidth() - 64, rowH - 4);
        const float peak = juce::jlimit(0.0f, 1.0f, meters.getPeak(ch));
        g.setColour(juce::Colours::white.withAlpha(0.70f));
        g.drawText("Ch " + juce::String(ch + 1), row.removeFromLeft(54), juce::Justification::centredLeft);
        auto bar = row.reduced(2);
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.fillRoundedRectangle(bar.toFloat(), 4.0f);
        g.setColour(juce::Colour(0xffd6bcfa));
        g.fillRoundedRectangle(bar.withWidth(static_cast<int>(bar.getWidth() * peak)).toFloat(), 4.0f);
    }
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(28);
    titleLabel.setBounds(area.removeFromTop(44));
    statusLabel.setBounds(area.removeFromTop(32));
    auto buttonRow = area.removeFromTop(42);
    audioSettingsButton.setBounds(buttonRow.removeFromLeft(160));
    buttonRow.removeFromLeft(12);
    savePresetButton.setBounds(buttonRow.removeFromLeft(180));
}

void MainComponent::timerCallback()
{
    repaint();
}

void MainComponent::computeMeters(const juce::AudioBuffer<float>& buffer)
{
    const int channels = juce::jmin(buffer.getNumChannels(), meters.getNumChannels());
    for (int ch = 0; ch < channels; ++ch)
    {
        const float rms = buffer.getRMSLevel(ch, 0, buffer.getNumSamples());
        const float peak = buffer.getMagnitude(ch, 0, buffer.getNumSamples());
        meters.setLevel(ch, rms, peak);
    }
}
