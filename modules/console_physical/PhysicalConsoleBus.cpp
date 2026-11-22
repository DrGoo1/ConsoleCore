#include "PhysicalConsoleBus.h"

namespace console
{
    namespace dsp = tape::dsp;

    bool PhysicalConsoleBus::prepare(double sampleRate, int numStems, const StageSpec& channelSpec, const StageSpec& busSpec, const BusRunSettings& settings)
    {
        ready = false;
        const double fs = sampleRate > 0.0 ? sampleRate : 48000.0;
        if (numStems <= 0 || numStems > kMaxStems)
            return false;
        if (channelSpec.referenceDbfs != busSpec.referenceDbfs || channelSpec.nominalLevelDbu != busSpec.nominalLevelDbu)
            return false;
        if (!channelModel.calibrate(channelSpec, 4.0 * fs) || !busModel.calibrate(busSpec, 4.0 * fs))
            return false;

        stems.assign(static_cast<std::size_t>(numStems), Stem {});
        for (auto& stem : stems)
            for (auto& side : stem.side)
            {
                side.up.prepare();
                channelModel.configure(side.state);
            }
        std::uint32_t seed = 0x13579bdu;
        for (auto& o : out)
        {
            o.down.prepare();
            busModel.configure(o.state);
            o.noise.state = seed;
            seed = seed * 1664525u + 1013904223u;
        }

        const double channelDrive = dsp::dbToGain(settings.channelDriveDb), busDrive = dsp::dbToGain(settings.busDriveDb);
        inputGain = channelDrive / dsp::dbToGain(channelSpec.referenceDbfs);
        afterChannel = busDrive / channelDrive;
        outputGain = dsp::dbToGain(busSpec.referenceDbfs) / busDrive;

        // Uncorrelated noise from every channel stage plus the bus stage, added once at the output.
        const double channelNoise = stageNoiseGain(channelSpec, fs), busNoise = stageNoiseGain(busSpec, fs);
        noiseGain = settings.noiseEnabled ? std::sqrt(numStems * channelNoise * channelNoise + busNoise * busNoise) : 0.0;
        latency = stagePathLatency(fs, 1.0 / std::min(channelSpec.hfCornerHz, 1.8 * fs) + 1.0 / std::min(busSpec.hfCornerHz, 1.8 * fs));
        ready = true;
        return true;
    }

    void PhysicalConsoleBus::reset()
    {
        for (auto& stem : stems)
            for (auto& side : stem.side)
            {
                side.up.reset();
                side.state.reset();
            }
        for (auto& o : out)
        {
            o.down.reset();
            o.state.reset();
        }
    }

    void PhysicalConsoleBus::process(const float* const* in, int numStems, float* outLeft, float* outRight, int numSamples)
    {
        if (!ready || outLeft == nullptr || outRight == nullptr || numSamples <= 0)
            return;
        const int count = in != nullptr ? std::min(numStems, static_cast<int>(stems.size())) : 0;
        float* outs[2] = { outLeft, outRight };
        for (int n = 0; n < numSamples; ++n)
            for (int c = 0; c < 2; ++c)
            {
                double sum[4] = { 0.0, 0.0, 0.0, 0.0 };
                for (int i = 0; i < count; ++i)
                {
                    const float* data = in[2 * i + c];
                    Side& side = stems[static_cast<std::size_t>(i)].side[c];
                    double over[4];
                    side.up.up(data != nullptr ? dsp::finiteOrZero(static_cast<double>(data[n])) * inputGain : 0.0, over);
                    for (int j = 0; j < 4; ++j)
                        sum[j] += channelModel.run(side.state, over[j]);
                }
                Output& o = out[c];
                for (int j = 0; j < 4; ++j)
                    sum[j] = busModel.run(o.state, sum[j] * afterChannel);
                double y = o.down.down(sum);
                if (noiseGain > 0.0)
                    y += noiseGain * o.noise.next();
                outs[c][n] = static_cast<float>(dsp::finiteOrZero(y * outputGain));
            }
    }
}
