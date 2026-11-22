#include "PhysicalConsoleStage.h"

#include <algorithm>
#include <cmath>

namespace console
{
    namespace dsp = tape::dsp;
    using dsp::kPi;

    namespace
    {
        constexpr double kFluxHz = 5.0;  // core flux follows the integral of the voltage
    }

    void PhysicalConsoleStage::Stage::reset()
    {
        for (auto* f : { &inFlux, &outFlux, &inHp, &outHp })
            f->reset();
        hf.reset();
    }

    void PhysicalConsoleStage::configure(Stage& stage) const
    {
        stage.inFlux.setLowpass(kFluxHz, fsOs);
        stage.outFlux.setLowpass(kFluxHz, fsOs);
        stage.inHp.setHighpass(spec.lfCornerHz, fsOs);
        stage.outHp.setHighpass(spec.lfCornerHz, fsOs);
        stage.hf.setLowpass(std::min(spec.hfCornerHz, 0.45 * fsOs), 0.7071, fsOs);
        stage.reset();
    }

    inline double PhysicalConsoleStage::run(Stage& s, double x, const Curve& c) const
    {
        if (spec.inputTransformer)
        {
            // Magnetising nonlinearity: a bounded cubic of the core flux, largest at low frequencies.
            const double phi = s.inFlux.process(x);
            x = s.inHp.process(x - phi * std::min(c.k * phi * phi, 0.5));
        }

        // Amplifier: nearly straight until close to the ceiling, then a firm limit, plus a
        // small even-order term for the asymmetry of a single-ended stage.
        x = std::clamp(x, -1.0e3 * c.ceiling, 1.0e3 * c.ceiling);
        const double r = std::abs(x) / c.ceiling;
        double y = x;
        if (r > 1.0e-3)
            y = x / std::pow(1.0 + std::pow(r, c.p), 1.0 / c.p);
        y += c.a2 * c.ceiling * (r * r) / (1.0 + r * r);

        if (spec.outputTransformer)
        {
            const double phi = s.outFlux.process(y);
            y -= phi * std::min(c.k * phi * phi, 0.5);
        }
        y = s.outHp.process(y);   // output coupling; also removes the offset of the even term
        return s.hf.process(y);
    }

    double PhysicalConsoleStage::measure(const Curve& c, double hz, double level, bool thirdOnly) const
    {
        Stage stage;
        configure(stage);
        // Short runs keep prepare() quick: whole cycles after a brief settle are enough here
        // because every filter in the stage sits far from the test frequencies.
        const int settle = static_cast<int>(std::round(4.0 * fsOs / hz));
        const int count = static_cast<int>(std::round(8.0 * fsOs / hz));
        double s[6] = {}, co[6] = {};
        for (int n = 0; n < settle + count; ++n)
        {
            const double ph = 2.0 * kPi * hz * n / fsOs;
            const double y = run(stage, level * std::sin(ph), c);
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
            if (!thirdOnly || h == 3)
                sum += s[h] * s[h] + co[h] * co[h];
        return std::sqrt(sum / f1);
    }

    bool PhysicalConsoleStage::prepare(double sampleRate, int numChannels, const StageSpec& stageSpec, const StageRunSettings& run)
    {
        ready = false;
        spec = stageSpec;
        settings = run;
        settings.stages = std::clamp(settings.stages, 1, 4);
        fs = sampleRate > 0.0 ? sampleRate : 48000.0;
        fsOs = 4.0 * fs;
        if (numChannels <= 0 || spec.clipCeilingDbu <= spec.maxOutputDbu || spec.maxOutputDbu <= spec.thdCalLevelDbu)
            return false;

        auto level = [&](double dbu) { return dsp::dbToGain(dbu - spec.nominalLevelDbu); };
        Curve c;
        c.ceiling = level(spec.clipCeilingDbu);
        c.a2 = 0.0;
        c.k = 0.0;

        // 1. Knee: distortion at the rated maximum output equals the figure taken to define it.
        double lo = 2.0, hi = 80.0;
        for (int i = 0; i < 24; ++i)
        {
            c.p = 0.5 * (lo + hi);
            (measure(c, 1000.0, level(spec.maxOutputDbu), false) > spec.thdPercentAtMaxOutput / 100.0 ? lo : hi) = c.p;
        }
        c.p = 0.5 * (lo + hi);

        // 2. Even-order amount: total distortion at the calibration level.
        lo = 0.0;
        hi = 0.5;
        for (int i = 0; i < 24; ++i)
        {
            c.a2 = 0.5 * (lo + hi);
            (measure(c, 1000.0, level(spec.thdCalLevelDbu), false) < spec.thdPercentAtCal / 100.0 ? lo : hi) = c.a2;
        }
        c.a2 = 0.5 * (lo + hi);

        // 3. Transformers: third harmonic at the low calibration frequency, amplifier held straight.
        if (spec.inputTransformer || spec.outputTransformer)
        {
            // Amplifier held straight so only the iron is measured. The search stays below the
            // point where the bounded term would stop growing.
            Curve iron = c;
            iron.a2 = 0.0;
            iron.ceiling = 1.0e9;
            const double amp = level(spec.thdCalLevelDbu);
            const double flux = amp * kFluxHz / spec.transformerCalHz;
            const double target = spec.transformerHd3PercentAtCal / 100.0;
            lo = 0.0;
            hi = 0.4 / (flux * flux);
            for (int i = 0; i < 30; ++i)
            {
                iron.k = 0.5 * (lo + hi);
                (measure(iron, spec.transformerCalHz, amp, true) < target ? lo : hi) = iron.k;
            }
            c.k = 0.5 * (lo + hi);
        }
        curve = c;

        channels.assign(static_cast<std::size_t>(numChannels), Channel {});
        std::uint32_t seed = 0x2468aceu;
        for (auto& ch : channels)
        {
            ch.os.prepare();
            ch.stages.assign(static_cast<std::size_t>(settings.stages), Stage {});
            for (auto& st : ch.stages)
                configure(st);
            ch.noise.state = seed;
            seed = seed * 1664525u + 1013904223u;
        }

        // White noise at the base rate whose 22 Hz to 22 kHz part equals the stated figure per stage.
        const double band = std::min(22000.0, 0.5 * fs) - 22.0;
        noiseGain = settings.noiseEnabled ? level(spec.noiseDbu) * std::sqrt(0.5 * fs / band) * std::sqrt(static_cast<double>(settings.stages)) : 0.0;

        const double drive = dsp::dbToGain(settings.driveDb);
        inputGain = drive / dsp::dbToGain(spec.referenceDbfs);
        outputGain = dsp::dbToGain(spec.referenceDbfs) / drive;
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
                        over[j] = run(st, over[j], curve);
                double y = ch.os.down(over);
                if (noiseGain > 0.0)
                    y += noiseGain * ch.noise.next();
                data[n] = static_cast<float>(dsp::finiteOrZero(y * outputGain));
            }
        }
    }
}
