#pragma once

#include <vector>
#include <atomic>

namespace consolecore
{
class LevelMeterState
{
public:
    void resize(int channels);
    void setLevel(int channel, float rms, float peak) noexcept;
    float getRms(int channel) const noexcept;
    float getPeak(int channel) const noexcept;
    int getNumChannels() const noexcept;

private:
    std::vector<std::atomic<float>> rmsValues;
    std::vector<std::atomic<float>> peakValues;
};
}
