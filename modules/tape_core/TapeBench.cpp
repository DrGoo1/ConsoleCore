#include "TapeBench.h"

#include "PhysicalTapeMachine.h"
#include "TapeDspBlocks.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace tape::bench
{
    using dsp::kPi;

    namespace
    {
        std::string fmt(const char* pattern, double a, double b = 0.0)
        {
            char buf[96];
            std::snprintf(buf, sizeof buf, pattern, a, b);
            return buf;
        }

        std::vector<float> tone(double hz, double levelDb, double refDbfs, double fs, int n)
        {
            std::vector<float> x(static_cast<std::size_t>(n));
            const double amp = dsp::dbToGain(levelDb + refDbfs);
            for (int i = 0; i < n; ++i)
                x[static_cast<std::size_t>(i)] = static_cast<float>(amp * std::sin(2.0 * kPi * hz * i / fs));
            return x;
        }

        // Amplitude of the component at hz over [start, start+count), by projection.
        double amplitudeAt(const std::vector<float>& y, double hz, double fs, int start, int count)
        {
            double s = 0.0, c = 0.0;
            for (int i = start; i < start + count; ++i)
            {
                const double ph = 2.0 * kPi * hz * i / fs;
                s += y[static_cast<std::size_t>(i)] * std::sin(ph);
                c += y[static_cast<std::size_t>(i)] * std::cos(ph);
            }
            return 2.0 * std::sqrt(s * s + c * c) / count;
        }

        int wholeCycles(double hz, double fs, double seconds)
        {
            const double cycles = std::max(8.0, std::floor(hz * seconds));
            return static_cast<int>(std::round(cycles * fs / hz));
        }
    }

    std::vector<float> runMono(const Rig& rig, const std::vector<float>& input)
    {
        PhysicalTapeMachine machine;
        const int block = 512;
        if (!machine.prepare(rig.sampleRate, block, 1, rig.spec, rig.run))
            return {};
        std::vector<float> out = input;
        for (std::size_t pos = 0; pos < out.size(); pos += block)
        {
            float* ch = out.data() + pos;
            machine.process(&ch, 1, static_cast<int>(std::min<std::size_t>(block, out.size() - pos)));
        }
        return out;
    }

    double responseDb(const Rig& rigIn, double hz, double levelDb)
    {
        Rig rig = rigIn;
        rig.run.flutterEnabled = false;
        rig.run.noiseEnabled = false;
        const double fs = rig.sampleRate;
        const int settle = static_cast<int>(fs * 0.8), count = wholeCycles(hz, fs, 0.5);
        const auto y = runMono(rig, tone(hz, levelDb, rig.spec.referenceDbfs, fs, settle + count));
        if (y.empty())
            return -200.0;
        const double in = dsp::dbToGain(levelDb + rig.spec.referenceDbfs);
        return dsp::gainToDb(amplitudeAt(y, hz, fs, settle, count) / in);
    }

    double hd3Percent(const Rig& rigIn, double hz, double levelDb)
    {
        Rig rig = rigIn;
        rig.run.flutterEnabled = false;
        rig.run.noiseEnabled = false;
        const double fs = rig.sampleRate;
        const int settle = static_cast<int>(fs * 0.8), count = wholeCycles(hz, fs, 0.5);
        const auto y = runMono(rig, tone(hz, levelDb, rig.spec.referenceDbfs, fs, settle + count));
        if (y.empty())
            return -1.0;
        const double fundamental = amplitudeAt(y, hz, fs, settle, count);
        return fundamental > 0.0 ? 100.0 * amplitudeAt(y, 3.0 * hz, fs, settle, count) / fundamental : 0.0;
    }

    double flutterRmsPercent(const Rig& rigIn, double loHz, double hiHz, double seconds)
    {
        Rig rig = rigIn;
        rig.run.noiseEnabled = false;
        const double fs = rig.sampleRate, f0 = 3150.0;
        const int n = static_cast<int>(fs * (seconds + 2.0));
        const auto y = runMono(rig, tone(f0, -10.0, rig.spec.referenceDbfs, fs, n));
        if (y.empty())
            return -1.0;

        // Quadrature demodulation, then phase difference gives the speed deviation.
        dsp::Biquad lpI1, lpI2, lpQ1, lpQ2, band;
        for (auto* b : { &lpI1, &lpI2, &lpQ1, &lpQ2 })
            b->setLowpass(1200.0, 0.7071, fs);
        band.setLowpass(hiHz, 0.7071, fs);
        dsp::FirstOrder hp;
        hp.setHighpass(loHz, fs);

        double prev = 0.0, sum = 0.0;
        int used = 0;
        const int skip = static_cast<int>(fs * 2.0);
        for (int i = 0; i < n; ++i)
        {
            const double ph = 2.0 * kPi * f0 * i / fs;
            const double re = lpI2.process(lpI1.process(y[static_cast<std::size_t>(i)] * std::cos(ph)));
            const double im = lpQ2.process(lpQ1.process(-y[static_cast<std::size_t>(i)] * std::sin(ph)));
            const double phase = std::atan2(im, re);
            double d = phase - prev;
            prev = phase;
            while (d > kPi) d -= 2.0 * kPi;
            while (d < -kPi) d += 2.0 * kPi;
            const double deviation = hp.process(band.process(d * fs / (2.0 * kPi * f0)));
            if (i >= skip)
            {
                sum += deviation * deviation;
                ++used;
            }
        }
        return used > 0 ? 100.0 * std::sqrt(sum / used) : -1.0;
    }

    double noiseDb(const Rig& rigIn)
    {
        Rig rig = rigIn;
        rig.run.noiseEnabled = true;
        rig.run.flutterEnabled = false;
        const double fs = rig.sampleRate;
        const int n = static_cast<int>(fs * 3.0);
        const auto y = runMono(rig, std::vector<float>(static_cast<std::size_t>(n), 0.0f));
        if (y.empty())
            return 0.0;
        dsp::Biquad lp;
        lp.setLowpass(std::min(20000.0, 0.45 * fs), 0.7071, fs);
        dsp::FirstOrder hp;
        hp.setHighpass(20.0, fs);
        double sum = 0.0;
        int used = 0;
        for (int i = 0; i < n; ++i)
        {
            const double v = hp.process(lp.process(y[static_cast<std::size_t>(i)]));
            if (i > n / 3)
            {
                sum += v * v;
                ++used;
            }
        }
        return dsp::gainToDb(std::sqrt(sum / used)) - rig.spec.referenceDbfs;
    }

    int measuredLatency(const Rig& rigIn)
    {
        Rig rig = rigIn;
        rig.run.flutterEnabled = false;
        rig.run.noiseEnabled = false;
        const int n = 8192;
        std::vector<float> x(static_cast<std::size_t>(n), 0.0f);
        dsp::Noise noise;
        noise.state = 7u;
        dsp::Biquad bandLp, bandHp;
        bandLp.setLowpass(6000.0, 0.7071, rig.sampleRate);
        bandHp.setHighpass(600.0, 0.7071, rig.sampleRate);
        const double amp = dsp::dbToGain(-30.0 + rig.spec.referenceDbfs);
        for (int i = 0; i < 4096; ++i)
            x[static_cast<std::size_t>(i)] = static_cast<float>(amp * bandHp.process(bandLp.process(noise.next())));
        const auto y = runMono(rig, x);
        if (y.empty())
            return -1;
        int best = 0;
        double bestValue = -1.0;
        for (int lag = 0; lag < 600; ++lag)
        {
            double acc = 0.0;
            for (int i = 0; i + lag < n && i < 4096; ++i)
                acc += x[static_cast<std::size_t>(i)] * y[static_cast<std::size_t>(i + lag)];
            if (acc > bestValue)
            {
                bestValue = acc;
                best = lag;
            }
        }
        return best;
    }

    std::vector<Row> report(const MachineSpec& spec, double ips, double sampleRate)
    {
        std::vector<Row> rows;
        const SpeedSpec* speed = spec.findSpeed(ips);
        if (speed == nullptr)
            return rows;
        Rig rig;
        rig.spec = spec;
        rig.sampleRate = sampleRate;
        rig.run.ips = ips;
        rig.run.linearizerEnabled = spec.hasLinearizer;

        const double ref = responseDb(rig, speed->referenceHz);
        rows.push_back({ fmt("gain at %.0f Hz", speed->referenceHz), fmt("%+.2f dB", ref), "within 0.5 dB of unity", "design", true, std::abs(ref) <= 0.5 });

        for (const auto& band : speed->publishedResponse)
        {
            double hi = -1e9, lo = 1e9;
            const int points = 13;
            // A band that runs past what the sample rate can carry is checked up to 0.45 fs.
            const double top = std::min(band.hiHz, 0.45 * rig.sampleRate);
            if (top <= band.loHz)
                continue;
            for (int i = 0; i < points; ++i)
            {
                const double hz = band.loHz * std::pow(top / band.loHz, static_cast<double>(i) / (points - 1));
                const double d = responseDb(rig, hz) - ref;
                hi = std::max(hi, d);
                lo = std::min(lo, d);
            }
            Row r;
            r.name = fmt("response %.0f Hz to %.0f Hz", band.loHz, top) + (top < band.hiHz ? fmt(" (published to %.0f)", band.hiHz) : std::string());
            r.measured = fmt("%+.2f / %+.2f dB", hi, lo);
            r.limit = fmt("+%.1f / -%.1f dB", band.plusDb, band.minusDb);
            r.basis = "published";
            r.hasLimit = true;
            r.pass = hi <= band.plusDb && lo >= -band.minusDb;
            rows.push_back(r);
        }
        if (speed->publishedResponse.empty())
            for (double hz : { 30.0, 50.0, 100.0, 10000.0, 15000.0, 20000.0 })
                rows.push_back({ fmt("response at %.0f Hz", hz), fmt("%+.2f dB", responseDb(rig, hz) - ref), "", "no published tolerance in the profile", false, true });

        {
            Rig off = rig;
            off.run.linearizerEnabled = false;
            const double hd = hd3Percent(off, 1000.0, spec.hd3CalLevelDb);
            const bool published = spec.evidence.count("tape.hd3PercentAtCal") && spec.evidence.at("tape.hd3PercentAtCal").rfind("published", 0) == 0;
            rows.push_back({ fmt("third harmonic, 1 kHz at +%.0f dB", spec.hd3CalLevelDb) + (spec.hasLinearizer ? ", linearizer off" : ""),
                             fmt("%.2f %%", hd), fmt("%.1f %% +/- 0.3", spec.hd3PercentAtCal), published ? "published" : "assumed",
                             published, std::abs(hd - spec.hd3PercentAtCal) <= 0.3 });
            if (spec.hasLinearizer)
            {
                const double on = hd3Percent(rig, 1000.0, spec.hd3CalLevelDb);
                rows.push_back({ fmt("third harmonic, 1 kHz at +%.0f dB, linearizer on", spec.hd3CalLevelDb), fmt("%.2f %%", on),
                                 "under 0.8 %", "published", true, on < 0.8 });
            }
            rows.push_back({ "third harmonic, 1 kHz at operating level", fmt("%.2f %%", hd3Percent(rig, 1000.0, 0.0)), "", "follows from the calibration", false, true });
        }

        if (spec.outputTransformer || spec.inputTransformer)
        {
            Rig clean = rig;
            clean.run.inputDriveDb = 0.0;
            const double hd = hd3Percent(clean, spec.transformerCalHz, 0.0);
            rows.push_back({ fmt("third harmonic, %.0f Hz at operating level", spec.transformerCalHz), fmt("%.3f %%", hd), "", "transformer and tape together", false, true });
        }

        {
            const double low = flutterRmsPercent(rig, 0.5, 300.0);
            Row r { "flutter, 0.5 Hz to 300 Hz", fmt("%.4f %% rms", low), "", "derived", false, true };
            if (speed->publishedFlutterMaxPercent > 0.0)
            {
                r.limit = fmt("at most %.3f %%", speed->publishedFlutterMaxPercent);
                r.basis = "published";
                r.hasLimit = true;
                r.pass = low <= speed->publishedFlutterMaxPercent;
            }
            rows.push_back(r);
        }

        rows.push_back({ "noise, 20 Hz to 20 kHz (when enabled)", fmt("%.1f dB re operating level", noiseDb(rig)), fmt("target -%.0f dB", speed->snrDb),
                         spec.evidence.count("speeds.snrDb") ? spec.evidence.at("speeds.snrDb").substr(0, spec.evidence.at("speeds.snrDb").find(':')) : std::string("assumed"), false, true });
        return rows;
    }
}
