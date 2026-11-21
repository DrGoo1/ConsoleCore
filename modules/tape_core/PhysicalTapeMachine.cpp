#include "PhysicalTapeMachine.h"

#include <cmath>

namespace tape
{
    using dsp::kPi;

    namespace
    {
        constexpr double kMetresPerInch = 0.0254;
        constexpr double kRecordCurveCapHz = 60000.0;  // keeps the record curve and its inverse finite
        constexpr double kLfBoostCapDb = 12.0;
        constexpr double kPreEmphasisCapDb = 15.0;
    }

    void PhysicalTapeMachine::Channel::reset()
    {
        for (auto* f : { &inHp, &outHp, &inXfmrFlux, &outXfmrFlux, &recLf, &recHf, &preEmph, &waveLoss, &repHf, &repLf })
            f->reset();
        for (auto* b : { &hfLimit, &bump, &bumpDip, &bumpHp })
            b->reset();
        os.reset();
        delay.reset();
    }

    void PhysicalTapeMachine::configureCore(Core& core, const SpeedSpec& speed, double rate) const
    {
        // Standard record flux curve. High-frequency corner from the time constant;
        // the response is capped far above the audio band so its inverse exists.
        const double hfHz = 1.0e6 / (2.0 * kPi * speed.hfTimeConstantUs);
        core.recHf.setShelf(kRecordCurveCapHz, hfHz, 1.0, rate);
        core.repHf.setShelf(hfHz, kRecordCurveCapHz, 1.0, rate);

        if (speed.lfTimeConstantUs > 0.0)
        {
            const double lfHz = 1.0e6 / (2.0 * kPi * speed.lfTimeConstantUs);
            const double g = dsp::dbToGain(kLfBoostCapDb);
            core.recLf.setShelf(lfHz, lfHz / g, g, rate);         // boost below lfHz, unity above
            core.repLf.setShelf(lfHz / g, lfHz, 1.0 / g, rate);   // exact inverse
        }
        else
        {
            core.recLf = dsp::FirstOrder {};
            core.repLf = dsp::FirstOrder {};
        }

        // Short-wavelength loss of the coating: (1 - e^-x)/x with x = 2*pi*thickness/wavelength
        // is 3 dB down at x = 0.76. The record amplifier pre-emphasises to compensate.
        const double speedMps = speed.ips * kMetresPerInch;
        const double lossHz = 0.76 * speedMps / (2.0 * kPi * machine.coatingThicknessMicrons * 1.0e-6);
        const double capHz = lossHz * dsp::dbToGain(kPreEmphasisCapDb);
        core.preEmph.setShelf(lossHz, capHz, 1.0, rate);
        core.waveLoss.setShelf(capHz, lossHz, 1.0, rate);
    }

    // Third harmonic of a 1 kHz tone through the core, as a fraction of the fundamental.
    // `phasor`, when given, receives the harmonic's sine and cosine parts so two
    // measurements can be compared for polarity.
    double PhysicalTapeMachine::measureHd3(Core core, double levelDb, double rate, double* phasor) const
    {
        const double f = 1000.0, amp = dsp::dbToGain(levelDb);
        const int settle = static_cast<int>(rate * 0.02);
        const int periods = 20;
        const int count = static_cast<int>(std::round(periods * rate / f));
        double s1 = 0.0, c1 = 0.0, s3 = 0.0, c3 = 0.0;
        for (int n = 0; n < settle + count; ++n)
        {
            const double ph = 2.0 * kPi * f * n / rate;
            double x = amp * std::sin(ph);
            x = core.recHf.process(core.recLf.process(x));
            x = core.waveLoss.process(core.saturate(core.preEmph.process(x)));
            x = core.repLf.process(core.repHf.process(x));
            if (n >= settle)
            {
                s1 += x * std::sin(ph);
                c1 += x * std::cos(ph);
                s3 += x * std::sin(3.0 * ph);
                c3 += x * std::cos(3.0 * ph);
            }
        }
        const double fundamental = std::sqrt(s1 * s1 + c1 * c1);
        if (phasor != nullptr)
        {
            phasor[0] = s3;
            phasor[1] = c3;
        }
        return fundamental > 0.0 ? std::sqrt(s3 * s3 + c3 * c3) / fundamental : 0.0;
    }

    void PhysicalTapeMachine::calibrateSaturation(const SpeedSpec& speed, double rate, bool linearizerOn)
    {
        Core core;
        configureCore(core, speed, rate);

        // Drive so the unlinearised tape shows the published third harmonic at the calibration level.
        const double target = machine.hd3PercentAtCal / 100.0;
        double lo = 1.0e-3, hi = 3.0;
        for (int i = 0; i < 40; ++i)
        {
            core.drive = 0.5 * (lo + hi);
            core.lin = 0.0;
            (measureHd3(core, machine.hd3CalLevelDb, rate) < target ? lo : hi) = core.drive;
        }
        satDrive = 0.5 * (lo + hi);

        linAmount = 0.0;
        if (linearizerOn && machine.hasLinearizer)
        {
            const double linTarget = machine.linearizerHd3Percent / 100.0;
            core.drive = satDrive;
            core.lin = 0.0;
            double reference[2] = { 0.0, 0.0 };
            measureHd3(core, machine.hd3CalLevelDb, rate, reference);

            // The linearizer reduces the harmonic and, pushed further, inverts it. The
            // filters do not change with the amount, so polarity is read against the
            // uncorrected harmonic.
            // A cubic correction of exactly 1 cancels the cubic term but leaves the
            // third harmonic produced by the fifth-order term, so the range runs past 1.
            double a = 0.0, b = 1.8;
            for (int i = 0; i < 40; ++i)
            {
                core.lin = 0.5 * (a + b);
                double now[2] = { 0.0, 0.0 };
                const double magnitude = measureHd3(core, machine.hd3CalLevelDb, rate, now);
                const bool sameSign = now[0] * reference[0] + now[1] * reference[1] >= 0.0;
                ((sameSign && magnitude > linTarget) ? a : b) = core.lin;
            }
            linAmount = 0.5 * (a + b);
        }
    }

    bool PhysicalTapeMachine::prepare(double sampleRate, int maxBlockSize, int numChannels, const MachineSpec& spec, const RunSettings& settings)
    {
        ready = false;
        machine = spec;
        run = settings;
        fs = sampleRate > 0.0 ? sampleRate : 48000.0;
        fsOs = 4.0 * fs;

        const SpeedSpec* speed = machine.findSpeed(run.ips);
        if (speed == nullptr || numChannels <= 0 || maxBlockSize <= 0)
            return false;

        calibrateSaturation(*speed, fsOs, run.linearizerEnabled);

        Core core;
        configureCore(core, *speed, fsOs);
        const double speedMps = speed->ips * kMetresPerInch;
        recordLossHz = 0.76 * speedMps / (2.0 * kPi * machine.coatingThicknessMicrons * 1.0e-6);
        bumpHz = speed->ips / machine.headBumpWavelengthInches;

        // Transformer: third harmonic rises as level^2 / frequency^3. Solve for the constant
        // that gives the stated figure at the stated frequency and the maximum output level.
        outputLimit = dsp::dbToGain(machine.maxOutputDb);
        const double ratio = kXfmrFluxHz / machine.transformerCalHz;
        xfmrK = 4.0 * (machine.transformerHd3Percent / 100.0) / (outputLimit * outputLimit * ratio * ratio * ratio);

        channels.assign(static_cast<std::size_t>(numChannels), Channel {});
        std::uint32_t seed = 0x1234567u;
        for (auto& ch : channels)
        {
            ch.inHp.setHighpass(machine.electronicsHighpassHz, fs);
            ch.outHp.setHighpass(machine.electronicsHighpassHz, fs);
            ch.inXfmrFlux.setLowpass(kXfmrFluxHz, fs);
            ch.outXfmrFlux.setLowpass(kXfmrFluxHz, fs);
            ch.os.prepare();
            ch.recLf = core.recLf;
            ch.recHf = core.recHf;
            ch.preEmph = core.preEmph;
            ch.waveLoss = core.waveLoss;
            ch.repHf = core.repHf;
            ch.repLf = core.repLf;
            ch.hfLimit.setLowpass(speed->playbackHfCornerHz, 0.7071, fsOs);
            ch.bump.setPeak(bumpHz, machine.headBumpGainDb, 1.2, fs);
            ch.bumpDip.setPeak(2.1 * bumpHz, -0.35 * machine.headBumpGainDb, 1.4, fs);
            ch.bumpHp.setHighpass(0.35 * bumpHz, 0.7071, fs);
            ch.delay.prepare(4096);
            ch.noise.state = seed;
            seed = seed * 747796405u + 2891336453u;
        }

        // Transport. Rotation-locked components scale with tape speed.
        sines.clear();
        double weightSq = 0.0;
        for (const auto& c : machine.flutterComponents)
            weightSq += c.weight * c.weight;
        const double lowRms = run.flutterEnabled ? speed->flutterLowBandRmsPercent / 100.0 : 0.0;
        for (std::size_t i = 0; i < machine.flutterComponents.size() && weightSq > 0.0; ++i)
        {
            const auto& c = machine.flutterComponents[i];
            SineComponent s;
            s.increment = 2.0 * kPi * c.hzAt15Ips * (speed->ips / 15.0) / fsOs;
            s.amplitude = std::sqrt(2.0) * lowRms * c.weight / std::sqrt(weightSq);
            s.phase = 2.0 * kPi * 0.61803398875 * static_cast<double>(i + 1);  // fixed, spread start phases
            sines.push_back(s);
        }
        scrapeHp.setHighpass(300.0, fsOs);
        scrapeLp.setLowpass(5000.0, fsOs);
        scrapeNoise.state = 0xBEEF1u;
        {
            // Measure the band-limited noise so the scrape component can be set in real units.
            double sum = 0.0;
            const int n = 1 << 16;
            for (int i = 0; i < n; ++i)
            {
                const double v = scrapeLp.process(scrapeHp.process(scrapeNoise.next()));
                sum += v * v;
            }
            const double rms = std::sqrt(sum / n);
            scrapeGain = (run.flutterEnabled && rms > 0.0) ? (speed->scrapeFlutterRmsPercent / 100.0) / rms : 0.0;
        }
        delayLeak = 1.0 - 2.0 * kPi * 0.2 / fsOs;
        delayBlock.assign(static_cast<std::size_t>(4 * maxBlockSize), kBaseDelayOs);

        // Tape noise: white at the tape, shaped by the reproduce chain. Scale to the stated
        // unweighted signal-to-noise ratio by measuring the chain's noise gain once.
        noiseGain = 0.0;
        if (run.noiseEnabled)
        {
            Core probe = core;
            dsp::Biquad limit, audioBand;
            limit.setLowpass(speed->playbackHfCornerHz, 0.7071, fsOs);
            audioBand.setLowpass(20000.0, 0.7071, fsOs);
            dsp::Noise n;
            n.state = 99u;
            double sum = 0.0;
            const int count = 1 << 16;
            for (int i = 0; i < count; ++i)
            {
                const double v = audioBand.process(limit.process(probe.repLf.process(probe.repHf.process(n.next()))));
                sum += v * v;
            }
            const double rms = std::sqrt(sum / count);
            noiseGain = rms > 0.0 ? dsp::dbToGain(-speed->snrDb) / rms : 0.0;
        }

        const double drive = dsp::dbToGain(run.inputDriveDb);
        inputGain = drive / dsp::dbToGain(machine.referenceDbfs);
        outputGain = dsp::dbToGain(machine.referenceDbfs) / drive;
        latency = dsp::Oversampler4x::latencySamples() + static_cast<int>(kBaseDelayOs / 4.0);

        reset();
        ready = true;
        return true;
    }

    void PhysicalTapeMachine::reset()
    {
        for (auto& ch : channels)
            ch.reset();
        scrapeHp.reset();
        scrapeLp.reset();
        scrapeNoise.state = 0xBEEF1u;
        delayAccumulator = 0.0;
        std::uint32_t seed = 0x1234567u;
        for (auto& ch : channels)
        {
            ch.noise.state = seed;
            seed = seed * 747796405u + 2891336453u;
        }
        for (std::size_t i = 0; i < sines.size(); ++i)
            sines[i].phase = 2.0 * kPi * 0.61803398875 * static_cast<double>(i + 1);
    }

    inline double PhysicalTapeMachine::transformer(dsp::FirstOrder& flux, double x) const
    {
        // Core flux follows the integral of the voltage, so it is largest at low frequencies.
        // The magnetising nonlinearity subtracts a bounded cubic term of that flux.
        const double phi = flux.process(x);
        const double bend = std::min(xfmrK * phi * phi, 0.5);
        return x - phi * bend;
    }

    void PhysicalTapeMachine::process(float* const* io, int numChannels, int numSamples)
    {
        if (!ready || io == nullptr || numSamples <= 0)
            return;
        const int count = std::min(numChannels, static_cast<int>(channels.size()));
        const int maxFrames = static_cast<int>(delayBlock.size()) / 4;

        // Blocks longer than the prepared size are handled in pieces.
        for (int offset = 0; offset < numSamples; offset += maxFrames)
        {
            const int frames = std::min(maxFrames, numSamples - offset);

            // One speed deviation for the whole transport, integrated into a delay.
            for (int i = 0; i < 4 * frames; ++i)
            {
                double deviation = scrapeGain * scrapeLp.process(scrapeHp.process(scrapeNoise.next()));
                for (auto& s : sines)
                {
                    deviation += s.amplitude * std::sin(s.phase);
                    s.phase += s.increment;
                    if (s.phase > 2.0 * kPi)
                        s.phase -= 2.0 * kPi;
                }
                delayAccumulator = delayAccumulator * delayLeak + deviation;
                delayBlock[static_cast<std::size_t>(i)] = std::clamp(kBaseDelayOs + delayAccumulator, 2.0, 3800.0);
            }

            for (int c = 0; c < count; ++c)
            {
                if (io[c] == nullptr)
                    continue;
                float* data = io[c] + offset;
                Channel& ch = channels[static_cast<std::size_t>(c)];
                for (int n = 0; n < frames; ++n)
                {
                    double x = dsp::finiteOrZero(static_cast<double>(data[n])) * inputGain;
                    x = ch.inHp.process(x);
                    if (machine.inputTransformer)
                        x = transformer(ch.inXfmrFlux, x);

                    double over[4];
                    ch.os.up(x, over);
                    for (int j = 0; j < 4; ++j)
                    {
                        double u = ch.recHf.process(ch.recLf.process(over[j]));
                        u = ch.preEmph.process(u);
                        const double v = std::clamp(satDrive * u, -1.0, 1.0);
                        u = std::tanh(satDrive * (u + linAmount * v * v * v / (3.0 * satDrive))) / satDrive;
                        u = ch.waveLoss.process(u);
                        if (noiseGain > 0.0)
                            u += noiseGain * ch.noise.next();
                        ch.delay.push(u);
                        u = ch.delay.read(delayBlock[static_cast<std::size_t>(4 * n + j)]);
                        u = ch.repLf.process(ch.repHf.process(u));
                        over[j] = ch.hfLimit.process(u);
                    }
                    double y = ch.os.down(over);

                    y = ch.bumpHp.process(ch.bumpDip.process(ch.bump.process(y)));
                    if (machine.outputTransformer)
                        y = transformer(ch.outXfmrFlux, y);
                    y = ch.outHp.process(y);

                    // Line amplifier: clean up to its rated maximum, then a soft ceiling.
                    const double mag = std::abs(y);
                    if (mag > outputLimit)
                    {
                        const double room = 0.3 * outputLimit;
                        y = std::copysign(outputLimit + room * std::tanh((mag - outputLimit) / room), y);
                    }
                    data[n] = static_cast<float>(dsp::finiteOrZero(y * outputGain));
                }
            }
        }
    }
}
