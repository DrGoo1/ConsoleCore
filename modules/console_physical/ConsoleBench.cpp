#include "ConsoleBench.h"
#include "PhysicalConsoleStage.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace console::bench
{
    namespace dsp = tape::dsp;
    using dsp::kPi;

    namespace
    {
        std::string fmt(const char* pattern, double a, double b = 0.0)
        {
            char buf[96];
            std::snprintf(buf, sizeof buf, pattern, a, b);
            return buf;
        }

        double amplitude(const Rig& rig, double dbu) { return dsp::dbToGain(dbu - rig.spec.nominalLevelDbu + rig.spec.referenceDbfs); }

        std::vector<float> tone(double hz, double amp, double fs, int n)
        {
            std::vector<float> x(static_cast<std::size_t>(n));
            for (int i = 0; i < n; ++i)
                x[static_cast<std::size_t>(i)] = static_cast<float>(amp * std::sin(2.0 * kPi * hz * i / fs));
            return x;
        }

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
            return static_cast<int>(std::round(std::max(8.0, std::floor(hz * seconds)) * fs / hz));
        }

        std::string basisOf(const StageSpec& spec, const char* key)
        {
            const auto it = spec.evidence.find(key);
            return it == spec.evidence.end() ? std::string("assumed") : it->second.substr(0, it->second.find(':'));
        }
    }

    std::vector<float> runMono(const Rig& rig, const std::vector<float>& input)
    {
        PhysicalConsoleStage stage;
        if (!stage.prepare(rig.sampleRate, 1, rig.spec, rig.run))
            return {};
        std::vector<float> out = input;
        for (std::size_t pos = 0; pos < out.size(); pos += 512)
        {
            float* ch = out.data() + pos;
            stage.process(&ch, 1, static_cast<int>(std::min<std::size_t>(512, out.size() - pos)));
        }
        return out;
    }

    double responseDb(const Rig& rigIn, double hz, double levelDbu)
    {
        Rig rig = rigIn;
        rig.run.noiseEnabled = false;
        const double fs = rig.sampleRate, amp = amplitude(rig, levelDbu);
        const int settle = static_cast<int>(fs * 0.8), count = wholeCycles(hz, fs, 0.5);
        const auto y = runMono(rig, tone(hz, amp, fs, settle + count));
        return y.empty() ? -200.0 : dsp::gainToDb(amplitudeAt(y, hz, fs, settle, count) / amp);
    }

    double thdPercent(const Rig& rigIn, double hz, double levelDbu, int only)
    {
        Rig rig = rigIn;
        rig.run.noiseEnabled = false;
        const double fs = rig.sampleRate;
        const int settle = static_cast<int>(fs * 0.8), count = wholeCycles(hz, fs, 0.5);
        const auto y = runMono(rig, tone(hz, amplitude(rig, levelDbu), fs, settle + count));
        if (y.empty())
            return -1.0;
        const double fundamental = amplitudeAt(y, hz, fs, settle, count);
        double sum = 0.0;
        for (int h = 2; h <= 5; ++h)
            if ((only == 0 || only == h) && h * hz < 0.5 * fs)
            {
                const double a = amplitudeAt(y, h * hz, fs, settle, count);
                sum += a * a;
            }
        return fundamental > 0.0 ? 100.0 * std::sqrt(sum) / fundamental : 0.0;
    }

    double noiseDbu(const Rig& rigIn)
    {
        Rig rig = rigIn;
        rig.run.noiseEnabled = true;
        const double fs = rig.sampleRate;
        const int n = static_cast<int>(fs * 3.0), skip = static_cast<int>(fs * 0.5);
        auto y = runMono(rig, std::vector<float>(static_cast<std::size_t>(n), 0.0f));
        if (y.empty())
            return 0.0;
        dsp::Biquad lp, hp;
        lp.setLowpass(std::min(22000.0, 0.49 * fs), 0.7071, fs);
        hp.setHighpass(22.0, 0.7071, fs);
        double sum = 0.0;
        for (int i = 0; i < n; ++i)
        {
            const double v = hp.process(lp.process(y[static_cast<std::size_t>(i)]));
            if (i >= skip)
                sum += v * v;
        }
        return dsp::gainToDb(std::sqrt(sum / (n - skip))) - rig.spec.referenceDbfs + rig.spec.nominalLevelDbu;
    }

    int measuredLatency(const Rig& rigIn)
    {
        Rig rig = rigIn;
        rig.run.noiseEnabled = false;
        const int n = 8192;
        std::vector<float> x(static_cast<std::size_t>(n));
        dsp::Noise noise;
        noise.state = 7u;
        dsp::Biquad lp;
        lp.setLowpass(6000.0, 0.7071, rig.sampleRate);
        for (auto& v : x)
            v = static_cast<float>(0.01 * lp.process(noise.next()));
        const auto y = runMono(rig, x);
        if (y.empty())
            return -1;
        int best = 0;
        double bestValue = -1e30;
        for (int lag = 0; lag < 400; ++lag)
        {
            double sum = 0.0;
            for (int i = 1000; i < n - 400; ++i)
                sum += x[static_cast<std::size_t>(i)] * y[static_cast<std::size_t>(i + lag)];
            if (sum > bestValue)
            {
                bestValue = sum;
                best = lag;
            }
        }
        return best;
    }

    std::vector<Row> report(const StageSpec& spec, double sampleRate)
    {
        Rig rig;
        rig.spec = spec;
        rig.sampleRate = sampleRate;
        std::vector<Row> rows;

        const double ref = responseDb(rig, 1000.0, spec.nominalLevelDbu);
        rows.push_back({ "gain at 1 kHz, nominal level", fmt("%+.3f dB", ref), "within 0.1 dB of unity", "design", true, std::abs(ref) <= 0.1 });

        for (const auto& band : spec.publishedResponse)
        {
            // The oversampling filters start to roll off above 0.42 of the sample rate (0.9 dB down
            // at 19.8 kHz when running at 44.1 kHz), so the band is checked up to there.
            const double top = std::min(band.hiHz, 0.42 * sampleRate);
            double hi = -1e9, lo = 1e9;
            for (int i = 0; i < 13; ++i)
            {
                const double d = responseDb(rig, band.loHz * std::pow(top / band.loHz, i / 12.0), spec.nominalLevelDbu) - ref;
                hi = std::max(hi, d);
                lo = std::min(lo, d);
            }
            rows.push_back({ fmt("response %.0f Hz to %.0f Hz", band.loHz, top), fmt("%+.2f / %+.2f dB", hi, lo),
                             fmt("+%.1f / -%.1f dB", band.plusDb, band.minusDb), "published", true, hi <= band.plusDb && lo >= -band.minusDb });
        }
        if (spec.hfCornerHz < 0.42 * sampleRate)
        {
            const double d = responseDb(rig, spec.hfCornerHz, spec.nominalLevelDbu) - ref;
            const bool published = basisOf(spec, "amplifier.hfCorner") == "published";
            rows.push_back({ fmt("response at %.0f Hz", spec.hfCornerHz), fmt("%+.2f dB", d), "-3 dB +/- 1", basisOf(spec, "amplifier.hfCorner"), published, std::abs(d + 3.0) <= 1.0 });
        }
        else
            rows.push_back({ fmt("response at %.0f Hz", spec.hfCornerHz), "not measurable at this sample rate", "", basisOf(spec, "amplifier.hfCorner"), false, true });

        const bool hasThdLimit = spec.publishedThdMaxPercent > 0.0;
        const double thdLevel = spec.publishedThdLevelDbu > 0.0 ? spec.publishedThdLevelDbu : spec.thdCalLevelDbu;
        double last = -1.0;
        for (double hz : { spec.publishedThdLoHz, 1000.0, spec.publishedThdHiHz })
        {
            if (hz == last)
                continue;
            last = hz;
            const double thd = thdPercent(rig, hz, thdLevel);
            rows.push_back({ fmt("distortion, %.0f Hz at +%.0f dBu", hz, thdLevel), fmt("%.3f %%", thd),
                             hasThdLimit ? fmt("at most %.2f %%", spec.publishedThdMaxPercent) : "", hasThdLimit ? "published" : "assumed",
                             hasThdLimit, thd <= spec.publishedThdMaxPercent });
        }
        rows.push_back({ fmt("second / third harmonic, 1 kHz at +%.0f dBu", spec.thdCalLevelDbu),
                         fmt("%.3f %% / %.3f %%", thdPercent(rig, 1000.0, spec.thdCalLevelDbu, 2), thdPercent(rig, 1000.0, spec.thdCalLevelDbu, 3)),
                         "", basisOf(spec, "amplifier.harmonicBalance"), false, true });
        rows.push_back({ "distortion, 1 kHz at nominal level", fmt("%.4f %%", thdPercent(rig, 1000.0, spec.nominalLevelDbu)), "", "follows from the calibration", false, true });
        rows.push_back({ fmt("distortion, 20 Hz at +%.0f dBu", spec.thdCalLevelDbu), fmt("%.2f %%", thdPercent(rig, 20.0, spec.thdCalLevelDbu)), "",
                         "outside the published band; follows from the transformer model", false, true });
        {
            const double thd = thdPercent(rig, 1000.0, spec.maxOutputDbu);
            const double out = responseDb(rig, 1000.0, spec.maxOutputDbu) + spec.maxOutputDbu;
            rows.push_back({ fmt("at +%.0f dBu in, 1 kHz", spec.maxOutputDbu), fmt("%.2f %% distortion, +%.1f dBu out", thd, out),
                             fmt("delivers at least +%.1f dBu", spec.maxOutputDbu - 0.5), basisOf(spec, "amplifier.maxOutput"),
                             basisOf(spec, "amplifier.maxOutput") == "published", out >= spec.maxOutputDbu - 0.5 });
        }
        rows.push_back({ "noise, 22 Hz to 22 kHz (when enabled)", fmt("%.1f dBu", noiseDbu(rig)), fmt("%.0f dBu +/- 1.5", spec.noiseDbu), basisOf(spec, "amplifier.noise"),
                         basisOf(spec, "amplifier.noise") == "published", std::abs(noiseDbu(rig) - spec.noiseDbu) <= 1.5 });
        return rows;
    }
}
