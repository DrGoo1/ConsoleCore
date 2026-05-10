#pragma once

#include <atomic>

namespace consolecore
{
class SharedRailState
{
public:
    void reset() noexcept
    {
        railSag.store(0.0f);
        activeChannels.store(0);
        frameCounter.store(0);
    }

    void setRailSag(float value) noexcept { railSag.store(value); }
    float getRailSag() const noexcept { return railSag.load(); }

    void setActiveChannels(int value) noexcept { activeChannels.store(value); }
    int getActiveChannels() const noexcept { return activeChannels.load(); }

    void advanceFrame() noexcept { frameCounter.fetch_add(1); }
    unsigned long long getFrameCounter() const noexcept { return frameCounter.load(); }

private:
    std::atomic<float> railSag { 0.0f };
    std::atomic<int> activeChannels { 0 };
    std::atomic<unsigned long long> frameCounter { 0 };
};
}
