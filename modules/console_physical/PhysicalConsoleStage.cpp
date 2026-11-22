#include "PhysicalConsoleStage.h"

namespace console
{
    namespace dsp = tape::dsp;
    using dsp::kPi;

    namespace
    {
        constexpr double kFluxHz = 5.0;  // core flux follows the integral of the voltage
    }

    void StageState::reset()
    {
        for (auto* f : { &inFlux, &outFlux, &inHp, &outHp })
            f->reset();
        hf.reset();
    }

    void StageModel::configure(StageState& state) const
    {
        state.inFlux.setLowpass(kFluxHz, fsOs);
        state.outFlux.setLowpass(kFluxHz, fsOs);
        state.inHp.setHighpass(spec.lfCornerHz, fsOs);
        state.outHp.setHighpass(spec.lfCornerHz, fsOs);
        state.hf.setLowpass(std::min(spec.hfCornerHz, 0.45 * fsOs), 0.7071, fsOs);
        state.reset();
    }

    double StageModel::measure(double hz, double amplitude, int only) const
    {
        StageState state;
        configure(state);
        // Short runs keep calibration quick: whole cycles after a brief settle are enough
        // because every filter in the stage sits far from the test frequencies.
        const int settle = static_cast<int>(std::round(4.0 * fsOs / hz));
        const int count = static_cast<int>(std::round(8.0 * fsOs / hz));
        double s[6] = {}, co[6] = {};
        for (int n = 0; n < settle + count; ++n)
        {
            const double ph = 2.0 * kPi * hz * n / fsOs;
            const double y = run(state, amplitude * std::sin(ph));
            if (n >= settle)
                for (int h = 1; h <= 5; ++h)
                {
                    s[h] += y * std::sin(h * ph);
                    co[h] += y * std::cos(h * ph);
                }
        }
        const double f1 = s[1] * s[1] + co[1] * co[1];
        if (f1 <= 0.0)
            return 0.0;
        double sum = 0.0;
        for (int h = 2; h <= 5; ++h)
            if (only == 0 || only == h)
                sum += s[h] * s[h] + co[h] * co[h];
        return std::sqrt(sum / f1);
    }

    bool StageModel::calibrate(const StageSpec& stageSpec, double oversampledRate)
    {
        spec = stageSpec;
        fsOs = oversampledRate;
        if (spec.clipCeilingDbu <= spec.maxOutputDbu || spec.maxOutputDbu <= spec.thdCalLevelDbu)
            return false;

        ceiling = level(spec.clipCeilingDbu);
        a2 = a3 = k = 0.0;

        // 1. Knee: distortion at the rated maximum output equals the figure tied to that rating.
        double lo = 2.0, hi = 80.0;
        for (int i = 0; i < 24; ++i)
        {
            p = 0.5 * (lo + hi);
            (measure(1000.0, level(spec.maxOutputDbu), 0) > spec.thdPercentAtMaxOutput / 100.0 ? lo : hi) = p;
        }
        p = 0.5 * (lo + hi);

        // 2. Below the knee: second harmonic for the even share of the calibration figure,
        //    then an odd term to bring the total up to it.
        const double cal = level(spec.thdCalLevelDbu), target = spec.thdPercentAtCal / 100.0;
        const double share = std::clamp(spec.evenHarmonicShare, 0.0, 1.0);
        lo = 0.0;
        hi = 0.5;
        for (int i = 0; i < 24; ++i)
        {
            a2 = 0.5 * (lo + hi);
            (measure(1000.0, cal, 2) < share * target ? lo : hi) = a2;
        }
        a2 = share > 0.0 ? 0.5 * (lo + hi) : 0.0;
        if (share < 1.0)
        {
            lo = 0.0;
            hi = 0.9;
            for (int i = 0; i < 24; ++i)
            {
                a3 = 0.5 * (lo + hi);
                (measure(1000.0, cal, 0) < target ? lo : hi) = a3;
            }
            a3 = 0.5 * (lo + hi);
        }

        // The terms from step 2 add a little at the rated output, so settle the knee again with them in place.
        lo = 2.0;
        hi = 80.0;
        for (int i = 0; i < 24; ++i)
        {
            p = 0.5 * (lo + hi);
            (measure(1000.0, level(spec.maxOutputDbu), 0) > spec.thdPercentAtMaxOutput / 100.0 ? lo : hi) = p;
        }
        p = 0.5 * (lo + hi);

        // 3. Transformers: third harmonic at the low calibration frequency with the amplifier
        //    held straight. The search stays below the point where the bounded term stops growing.
        if (spec.inputTransformer || spec.outputTransformer)
        {
            const double keepCeiling = ceiling, keepA2 = a2, keepA3 = a3;
            ceiling = 1.0e9;
            a2 = a3 = 0.0;
            const double flux = cal * kFluxHz / spec.transformerCalHz;
            const double ironTarget = spec.transformerHd3PercentAtCal / 100.0;
            lo = 0.0;
            hi = 0.4 / (flux * flux);
            for (int i = 0; i < 30; ++i)
            {
                k = 0.5 * (lo + hi);
                (measure(spec.transformerCalHz, cal, 3) < ironTarget ? lo : hi) = k;
            }
            k = 0.5 * (lo + hi);
            ceiling = keepCeiling;
            a2 = keepA2;
            a3 = keepA3;
        }
        return true;
    }

    int stagePathLatency(double sampleRate, double inverseCornerSum)
    {
        // Group delay of a second-order Butterworth low-pass at low frequency is sqrt(2)/(2*pi*fc) seconds.
        return static_cast<int>(std::lround(34.2 + sampleRate * 0.2251 * inverseCornerSum));
    }

    double stageNoiseGain(const StageSpec& spec, double sampleRate)
    {
        const double band = std::min(22000.0, 0.5 * sampleRate) - 22.0;
        return dsp::dbToGain(spec.noiseDbu - spec.nominalLevelDbu) * std::sqrt(0.5 * sampleRate / band);
    }

    bool PhysicalConsoleStage::prepare(double sampleRate, int numChannels, const StageSpec& spec, const StageRunSettings& run)
    {
        ready = false;
        settings = run;
        settings.stages = std::clamp(settings.stages, 1, 4);
        fs = sampleRate > 0.0 ? sampleRate : 48000.0;
        if (numChannels <= 0 || !model.calibrate(spec, 4.0 * fs))
            return false;

        channels.assign(static_cast<std::size_t>(numChannels), Channel {});
        std::uint32_t seed = 0x2468aceu;
        for (auto& ch : channels)
        {
            ch.os.prepare();
            ch.stages.assign(static_cast<std::size_t>(settings.stages), StageState {});
            for (auto& st : ch.stages)
                model.configure(st);
            ch.noise.state = seed;
            seed = seed * 1664525u + 1013904223u;
        }

        noiseGain = settings.noiseEnabled ? stageNoiseGain(spec, fs) * std::sqrt(static_cast<double>(settings.stages)) : 0.0;
        const double drive = dsp::dbToGain(settings.driveDb);
        inputGain = drive / dsp::dbToGain(spec.referenceDbfs);
        outputGain = dsp::dbToGain(spec.referenceDbfs) / drive;
        latency = stagePathLatency(fs, settings.stages / std::min(spec.hfCornerHz, 0.45 * 4.0 * fs));
        ready = true;
        return true;
    }

    void PhysicalConsoleStage::reset()
    {
        for (auto& ch : channels)
        {
            ch.os.reset();
            for (auto& st : ch.stages)
                st.reset();
        }
    }

    void PhysicalConsoleStage::process(float* const* io, int numChannels, int numSamples)
    {
        if (!ready || io == nullptr || numSamples <= 0)
            return;
        const int count = std::min(numChannels, static_cast<int>(channels.size()));
        for (int c = 0; c < count; ++c)
        {
            float* data = io[c];
            if (data == nullptr)
                continue;
            Channel& ch = channels[static_cast<std::size_t>(c)];
            for (int n = 0; n < numSamples; ++n)
            {
                double over[4];
                ch.os.up(dsp::finiteOrZero(static_cast<double>(data[n])) * inputGain, over);
                for (int j = 0; j < 4; ++j)
                    for (auto& st : ch.stages)
                        over[j] = model.run(st, over[j]);
                double y = ch.os.down(over);
                if (noiseGain > 0.0)
                    y += noiseGain * ch.noise.next();
                data[n] = static_cast<float>(dsp::finiteOrZero(y * outputGain));
            }
        }
    }
}
