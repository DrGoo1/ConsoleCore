#include "LevelMeterState.h"

namespace consolecore
{
void LevelMeterState::resize(int channels)
{
    rmsValues = std::vector<std::atomic<float>>(static_cast<size_t>(channels));
    peakValues = std::vector<std::atomic<float>>(static_cast<size_t>(channels));
    for (int i = 0; i < channels; ++i)
    {
        rmsValues[static_cast<size_t>(i)].store(0.0f);
        peakValues[static_cast<size_t>(i)].store(0.0f);
    }
}

void LevelMeterState::setLevel(int channel, float rms, float peak) noexcept
{
    if (channel < 0 || channel >= getNumChannels()) return;
    rmsValues[static_cast<size_t>(channel)].store(rms);
    peakValues[static_cast<size_t>(channel)].store(peak);
}

float LevelMeterState::getRms(int channel) const noexcept
{
    if (channel < 0 || channel >= getNumChannels()) return 0.0f;
    return rmsValues[static_cast<size_t>(channel)].load();
}

float LevelMeterState::getPeak(int channel) const noexcept
{
    if (channel < 0 || channel >= getNumChannels()) return 0.0f;
    return peakValues[static_cast<size_t>(channel)].load();
}

int LevelMeterState::getNumChannels() const noexcept
{
    return static_cast<int>(rmsValues.size());
}
}
