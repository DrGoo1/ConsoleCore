#include "MainComponent.h"

MainComponent::MainComponent()
{
    setOpaque(true);

    titleLabel.setText("ConsoleCore Phase 1", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::FontOptions(28.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    statusLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(statusLabel);

    audioSettingsButton.onClick = [this] { openAudioSettings(); };
    savePresetButton.onClick = [this] { saveDevicePreset(); };
    latencyProbeButton.onClick = [this] { armLatencyProbe(); };

    addAndMakeVisible(audioSettingsButton);
    addAndMakeVisible(savePresetButton);
    addAndMakeVisible(latencyProbeButton);

    setAudioChannels(64, 64);
    meters.resize(64);
    routing.setIdentity(64);
    startTimerHz(24);
    setSize(1040, 680);
    refreshStatusText();
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    lastSampleRate = sampleRate;
    lastBlockSize = samplesPerBlockExpected;
    engine.prepare(sampleRate, samplesPerBlockExpected, 64);
    latencyProbe.prepare(sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* buffer = bufferToFill.buffer;
    if (buffer == nullptr) return;

    const int numSamples = bufferToFill.numSamples;
    const int numChannels = buffer->getNumChannels();

    scratchOutput.setSize(numChannels, numSamples, false, false, true);
    routing.process(*buffer, scratchOutput, numSamples);
    buffer->makeCopyOf(scratchOutput, true);

    latencyProbe.injectPulse(*buffer, 0);
    latencyProbe.scanReturn(*buffer, 0);

    engine.process(*buffer);
    computeMeters(*buffer);
}

void MainComponent::releaseResources()
{
    engine.reset();
    scratchOutput.setSize(0, 0);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff111118));
    g.setColour(juce::Colour(0xff9f7aea));
    g.drawRoundedRectangle(getLocalBounds().reduced(16).toFloat(), 16.0f, 2.0f);

    auto meterArea = getLocalBounds().withTrimmedTop(160).reduced(28);
    const int channelsToShow = juce::jmin(16, meters.getNumChannels());
    const int rowH = 24;

    g.setColour(juce::Colours::white.withAlpha(0.85f));
    g.setFont(15.0f);
    g.drawText("Phase 1: CoreAudio/Loopback pass-through, routing matrix, meters, preset capture, latency probe", meterArea.removeFromTop(28), juce::Justification::centredLeft);

    for (int ch = 0; ch < channelsToShow; ++ch)
    {
        auto row = juce::Rectangle<int>(32, 198 + ch * rowH, getWidth() - 64, rowH - 5);
        const float peak = juce::jlimit(0.0f, 1.0f, meters.getPeak(ch));
        const float rms = juce::jlimit(0.0f, 1.0f, meters.getRms(ch));

        g.setColour(juce::Colours::white.withAlpha(0.70f));
        g.drawText("Ch " + juce::String(ch + 1), row.removeFromLeft(54), juce::Justification::centredLeft);
        auto bar = row.reduced(2);
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.fillRoundedRectangle(bar.toFloat(), 4.0f);
        g.setColour(juce::Colour(0xff805ad5));
        g.fillRoundedRectangle(bar.withWidth(static_cast<int>(bar.getWidth() * rms)).toFloat(), 4.0f);
        g.setColour(juce::Colour(0xffd6bcfa));
        g.fillRoundedRectangle(bar.withWidth(static_cast<int>(bar.getWidth() * peak)).withHeight(3).toFloat(), 2.0f);
    }

    g.setColour(juce::Colours::white.withAlpha(0.72f));
    g.setFont(13.0f);
    auto footer = getLocalBounds().reduced(32).removeFromBottom(42);
    if (latencyProbe.getDetectedLatencySamples() >= 0)
        g.drawText("Detected loopback latency: " + juce::String(latencyProbe.getDetectedLatencySamples()) + " samples", footer, juce::Justification::centredLeft);
    else if (latencyProbe.isArmed())
        g.drawText("Latency probe armed: route output channel 1 back to input channel 1 through Loopback to detect roundtrip.", footer, juce::Justification::centredLeft);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(28);
    titleLabel.setBounds(area.removeFromTop(44));
    statusLabel.setBounds(area.removeFromTop(32));
    auto buttonRow = area.removeFromTop(42);
    audioSettingsButton.setBounds(buttonRow.removeFromLeft(160));
    buttonRow.removeFromLeft(12);
    savePresetButton.setBounds(buttonRow.removeFromLeft(190));
    buttonRow.removeFromLeft(12);
    latencyProbeButton.setBounds(buttonRow.removeFromLeft(190));
}

void MainComponent::timerCallback()
{
    refreshStatusText();
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

void MainComponent::openAudioSettings()
{
    auto selector = std::make_unique<juce::AudioDeviceSelectorComponent>(deviceManager,
                                                                        0, 64,
                                                                        0, 64,
                                                                        true, true, true, false);
    selector->setSize(760, 560);
    juce::DialogWindow::LaunchOptions opts;
    opts.content.setOwned(selector.release());
    opts.dialogTitle = "ConsoleCore Audio Settings";
    opts.dialogBackgroundColour = juce::Colours::darkgrey;
    opts.escapeKeyTriggersCloseButton = true;
    opts.useNativeTitleBar = true;
    opts.resizable = true;
    opts.launchAsync();
}

void MainComponent::saveDevicePreset()
{
    consolecore::DevicePreset preset;
    if (auto* device = deviceManager.getCurrentAudioDevice())
    {
        preset.inputDeviceName = device->getName();
        preset.outputDeviceName = device->getName();
        preset.sampleRate = device->getCurrentSampleRate();
        preset.bufferSize = device->getCurrentBufferSizeSamples();
        preset.inputChannels = device->getActiveInputChannels().countNumberOfSetBits();
        preset.outputChannels = device->getActiveOutputChannels().countNumberOfSetBits();
    }
    auto folder = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("ConsoleCore");
    folder.createDirectory();
    const auto file = folder.getChildFile("ConsoleCore_DevicePreset.json");
    preset.saveToFile(file);
}

void MainComponent::armLatencyProbe()
{
    latencyProbe.arm();
}

void MainComponent::refreshStatusText()
{
    juce::String text = "No CoreAudio device selected.";
    if (auto* device = deviceManager.getCurrentAudioDevice())
    {
        text = "Device: " + device->getName()
            + " | SR " + juce::String(device->getCurrentSampleRate(), 0)
            + " | Buffer " + juce::String(device->getCurrentBufferSizeSamples())
            + " | In " + juce::String(device->getActiveInputChannels().countNumberOfSetBits())
            + " / Out " + juce::String(device->getActiveOutputChannels().countNumberOfSetBits());
    }
    statusLabel.setText(text, juce::dontSendNotification);
}
