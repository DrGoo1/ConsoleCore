#include "PhysicalStemMixRuntime.h"

#include <algorithm>
#include <cmath>

bool PhysicalStemMixRuntime::prepare(double sampleRate, int maxBlockSize, const StemMixConfig& cfg)
{
    ready = false;
    error = "";
    config = cfg;
    maxBlock = maxBlockSize > 0 ? maxBlockSize : 512;
    const int stems = config.numStems;
    if (stems <= 0 || stems > console::PhysicalConsoleBus::kMaxStems)
    {
        error = "stem count must be between 1 and 64";
        return false;
    }
    if (!config.stemUsesTape.empty() && static_cast<int>(config.stemUsesTape.size()) != stems)
    {
        error = "stemUsesTape must be empty or have one entry per stem";
        return false;
    }

    tapeLatency = 0;
    if (config.tapeEnabled)
    {
        int tapeStems = 0;
        for (int i = 0; i < stems; ++i)
            tapeStems += (config.stemUsesTape.empty() || config.stemUsesTape[static_cast<std::size_t>(i)]) ? 1 : 0;
        if (2 * tapeStems > config.tapeMachine.tracks)
        {
            error = "more stems are assigned to tape than the machine has tracks for (two tracks per stereo stem)";
            return false;
        }
        if (!machine.prepare(sampleRate, maxBlock, 2 * stems, config.tapeMachine, config.tapeRun))
        {
            error = "the tape machine has no entry for the requested speed";
            return false;
        }
        tapeLatency = machine.latencySamples();
    }

    if (!bus.prepare(sampleRate, stems, config.channelStage, config.busStage, config.busRun))
    {
        error = "the console bus could not be prepared (unusable stage profile, or channel and bus reference levels differ)";
        return false;
    }

    scratch.assign(static_cast<std::size_t>(2 * stems), std::vector<float>(static_cast<std::size_t>(maxBlock), 0.0f));
    tapePointers.assign(static_cast<std::size_t>(2 * stems), nullptr);
    busPointers.assign(static_cast<std::size_t>(2 * stems), nullptr);
    bypassDelays.assign(static_cast<std::size_t>(2 * stems), Delay {});
    for (int i = 0; i < stems; ++i)
    {
        const bool onTape = config.tapeEnabled && (config.stemUsesTape.empty() || config.stemUsesTape[static_cast<std::size_t>(i)]);
        for (int c = 0; c < 2; ++c)
        {
            const std::size_t index = static_cast<std::size_t>(2 * i + c);
            tapePointers[index] = onTape ? scratch[index].data() : nullptr;
            busPointers[index] = scratch[index].data();
            if (!onTape && tapeLatency > 0)
                bypassDelays[index].buffer.assign(static_cast<std::size_t>(tapeLatency), 0.0f);
        }
    }

    stemBus.resize(stems);
    meters = {};
    latency = tapeLatency + bus.latencySamples();
    ready = true;
    return true;
}

void PhysicalStemMixRuntime::reset()
{
    machine.reset();
    bus.reset();
    for (auto& d : bypassDelays)
    {
        std::fill(d.buffer.begin(), d.buffer.end(), 0.0f);
        d.position = 0;
    }
    meters = {};
}

void PhysicalStemMixRuntime::process(const float* const* in, int numStems, float* outLeft, float* outRight, int numSamples)
{
    if (!ready || outLeft == nullptr || outRight == nullptr || numSamples <= 0)
        return;
    const int stems = config.numStems;
    const int given = in != nullptr ? std::min(numStems, stems) : 0;
    float outPeak = 0.0f;
    int active = 0;
    for (int i = 0; i < stems; ++i)
        stemBus.stem(i).peak = 0.0f;

    for (int offset = 0; offset < numSamples; offset += maxBlock)
    {
        const int frames = std::min(maxBlock, numSamples - offset);

        // Copy in (inputs stay untouched), noting each stem's input peak.
        for (int i = 0; i < stems; ++i)
        {
            float peak = stemBus.stem(i).peak;
            for (int c = 0; c < 2; ++c)
            {
                const std::size_t index = static_cast<std::size_t>(2 * i + c);
                float* dst = scratch[index].data();
                const float* src = i < given ? in[index] : nullptr;
                if (src == nullptr)
                {
                    std::fill(dst, dst + frames, 0.0f);
                    continue;
                }
                for (int n = 0; n < frames; ++n)
                {
                    const float v = std::isfinite(src[offset + n]) ? src[offset + n] : 0.0f;
                    dst[n] = v;
                    peak = std::max(peak, std::abs(v));
                }
            }
            stemBus.stem(i).peak = peak;
        }

        if (config.tapeEnabled)
        {
            machine.process(tapePointers.data(), 2 * stems, frames);

            // Stems that skip the tape wait for the ones that did not.
            for (std::size_t index = 0; index < bypassDelays.size(); ++index)
            {
                Delay& d = bypassDelays[index];
                if (d.buffer.empty())
                    continue;
                float* data = scratch[index].data();
                for (int n = 0; n < frames; ++n)
                {
                    const float delayed = d.buffer[d.position];
                    d.buffer[d.position] = data[n];
                    data[n] = delayed;
                    d.position = d.position + 1 == d.buffer.size() ? 0 : d.position + 1;
                }
            }
        }

        bus.process(busPointers.data(), stems, outLeft + offset, outRight + offset, frames);
        for (int n = 0; n < frames; ++n)
            outPeak = std::max(outPeak, std::max(std::abs(outLeft[offset + n]), std::abs(outRight[offset + n])));
    }

    for (int i = 0; i < stems; ++i)
        active += stemBus.stem(i).peak > 1.0e-5f ? 1 : 0;
    meters.globalPeak = outPeak;
    meters.activeStemCount = active;
}
