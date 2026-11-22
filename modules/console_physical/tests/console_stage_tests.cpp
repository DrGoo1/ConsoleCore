// Tests for the physical console stage model. Standard library only.
//
// These check that the model is well behaved and sits inside figures taken from
// documentation. They cannot show that it sounds like any real console.

#include "ConsoleBench.h"
#include "ConsoleSpecJson.h"
#include "PhysicalConsoleStage.h"
#include "PhysicalConsoleBus.h"
#include "TapeSpecJson.h"
#include "../../integration/IntegratedAnalogPathEngine.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#ifndef CONSOLE_PROFILE_DIR
#define CONSOLE_PROFILE_DIR "console_profiles/stages"
#endif
#ifndef TAPE_PROFILE_DIR
#define TAPE_PROFILE_DIR "tape_profiles/machines"
#endif

namespace
{
    int failures = 0;

    void check(bool ok, const std::string& what)
    {
        std::printf("%s  %s\n", ok ? "ok  " : "FAIL", what.c_str());
        if (!ok)
            ++failures;
    }

    console::bench::Rig rig(const console::StageSpec& spec, double fs = 48000.0)
    {
        console::bench::Rig r;
        r.spec = spec;
        r.sampleRate = fs;
        return r;
    }
}

int main()
{
    console::StageSpec spec;
    std::string error;
    if (!console::loadStageSpecFile(std::string(CONSOLE_PROFILE_DIR) + "/british_class_a_discrete_line_1970.json", spec, error))
    {
        std::printf("FAIL  cannot load profile: %s\n", error.c_str());
        return 1;
    }

    // --- Profile -----------------------------------------------------------------------------
    {
        const auto problems = console::validateStageSpec(spec);
        for (const auto& p : problems)
            std::printf("      %s\n", p.c_str());
        check(problems.empty(), "profile is valid and every sound-shaping field has evidence");
        check(spec.status.find("not validated") != std::string::npos && spec.referenceModule.find("not a claim of equivalence") != std::string::npos,
              "profile says it is not validated and disclaims equivalence");
        console::StageSpec out;
        check(!console::parseStageSpec("{\"schema\":\"tapecore.machine.v2\"}", out, error) && !error.empty(), "a tape profile is not accepted as a console stage");
        console::StageSpec gap = spec;
        gap.evidence.erase("amplifier.harmonicBalance");
        check(!console::validateStageSpec(gap).empty(), "a field without evidence fails validation");
        console::StageSpec silly = spec;
        silly.clipCeilingDbu = silly.maxOutputDbu - 1.0;
        console::PhysicalConsoleStage refuse;
        check(!console::validateStageSpec(silly).empty() && !refuse.prepare(48000.0, 1, silly, {}) && !refuse.isReady(), "a ceiling below the rated output is refused");
    }

    // --- Published figures --------------------------------------------------------------------
    for (double fs : { 44100.0, 48000.0, 96000.0 })
    {
        bool allPass = true;
        int limits = 0;
        for (const auto& row : console::bench::report(spec, fs))
            if (row.hasLimit)
            {
                ++limits;
                allPass = allPass && row.pass;
                if (!row.pass)
                    std::printf("      %s: %s (limit %s)\n", row.name.c_str(), row.measured.c_str(), row.limit.c_str());
            }
        check(allPass && limits >= 6, "model is inside every published figure at " + std::to_string(static_cast<int>(fs)) + " Hz");
    }

    // --- Behaviour ------------------------------------------------------------------------------
    {
        const auto r = rig(spec);
        const double t4 = console::bench::thdPercent(r, 1000.0, 4.0), t20 = console::bench::thdPercent(r, 1000.0, 20.0), t27 = console::bench::thdPercent(r, 1000.0, 27.0);
        check(t4 < 0.02 && t4 < t20 && t20 < t27, "stage is clean at nominal level and distortion rises with level");
        check(console::bench::thdPercent(r, 1000.0, 16.0, 2) > 3.0 * console::bench::thdPercent(r, 1000.0, 16.0, 3), "below the knee the distortion is mostly second harmonic");
        check(console::bench::thdPercent(r, 1000.0, 28.0, 3) > 3.0 * console::bench::thdPercent(r, 1000.0, 28.0, 2), "into the knee the third harmonic takes over");
        check(console::bench::thdPercent(r, 20.0, 20.0) > 3.0 * console::bench::thdPercent(r, 1000.0, 20.0), "transformers distort more at low frequencies");
        const double hot = console::bench::responseDb(r, 1000.0, 40.0) + 40.0;
        check(hot <= spec.clipCeilingDbu + 2.2 && hot > spec.maxOutputDbu, "output never passes the ceiling (a clipped sine's fundamental reads up to 2.1 dB over it)");

        auto bare = spec;
        bare.inputTransformer = bare.outputTransformer = false;
        check(console::bench::thdPercent(rig(bare), 20.0, 20.0) < 0.5 * console::bench::thdPercent(r, 20.0, 20.0), "removing the transformers removes the low-frequency distortion");

        auto two = rig(spec);
        two.run.stages = 2;
        const double oneStage = console::bench::thdPercent(r, 1000.0, 20.0, 2), twoStages = console::bench::thdPercent(two, 1000.0, 20.0, 2);
        check(twoStages > 1.6 * oneStage && twoStages < 2.4 * oneStage, "two stages in series roughly double the second harmonic");
        check(std::abs(console::bench::responseDb(two, 1000.0, 4.0)) < 0.1, "two stages keep unity gain");

        auto driven = rig(spec);
        driven.run.driveDb = 12.0;
        check(console::bench::thdPercent(driven, 1000.0, 10.0) > 2.0 * console::bench::thdPercent(r, 1000.0, 10.0) && std::abs(console::bench::responseDb(driven, 1000.0, 4.0)) < 0.1,
              "drive adds distortion and is level-compensated");
    }

    // --- Engine contract --------------------------------------------------------------------------
    {
        for (double fs : { 48000.0, 96000.0 })
            for (int stages : { 1, 3 })
            {
                auto r = rig(spec, fs);
                r.run.stages = stages;
                console::PhysicalConsoleStage probe;
                probe.prepare(fs, 1, spec, r.run);
                const int measured = console::bench::measuredLatency(r);
                check(measured == probe.latencySamples(), "reported latency matches measured latency: " + std::to_string(probe.latencySamples()) + " samples, "
                                                              + std::to_string(stages) + " stage(s) at " + std::to_string(static_cast<int>(fs)) + " Hz (measured " + std::to_string(measured) + ")");
            }

        const auto quiet = console::bench::runMono(rig(spec), std::vector<float>(4096, 0.0f));
        double peak = 0.0;
        for (float v : quiet)
            peak = std::max(peak, static_cast<double>(std::abs(v)));
        check(peak == 0.0, "silence in gives silence out when noise is off");

        std::vector<float> nasty(4096);
        for (std::size_t i = 0; i < nasty.size(); ++i)
            nasty[i] = (i % 97 == 0) ? 1.0e6f : ((i % 31 == 0) ? -50.0f : 0.5f);
        nasty[100] = std::nanf("");
        nasty[200] = INFINITY;
        bool finite = true;
        double biggest = 0.0;
        for (float v : console::bench::runMono(rig(spec), nasty))
        {
            finite = finite && std::isfinite(v);
            biggest = std::max(biggest, static_cast<double>(std::abs(v)));
        }
        check(finite && biggest < 8.0, "extreme and non-finite input stays finite and bounded");

        std::vector<float> a(3000), b, c;
        for (std::size_t i = 0; i < a.size(); ++i)
            a[i] = 0.3f * static_cast<float>(std::sin(0.05 * i) + 0.5 * std::sin(0.31 * i));
        b = c = a;
        console::PhysicalConsoleStage x;
        x.prepare(48000.0, 1, spec, {});
        float* pb = b.data();
        x.process(&pb, 1, static_cast<int>(b.size()));
        x.reset();
        for (std::size_t pos = 0; pos < c.size(); pos += 100)
        {
            float* pc = c.data() + pos;
            x.process(&pc, 1, static_cast<int>(std::min<std::size_t>(100, c.size() - pos)));
        }
        double worst = 0.0;
        for (std::size_t i = 0; i < b.size(); ++i)
            worst = std::max(worst, static_cast<double>(std::abs(b[i] - c[i])));
        check(worst < 1e-7, "output is deterministic and independent of block size");

        console::PhysicalConsoleStage y;
        std::vector<float> untouched(64, 0.25f);
        float* pu = untouched.data();
        y.process(&pu, 1, 64);
        check(untouched[10] == 0.25f, "an unprepared stage passes audio through unchanged");
    }

    // --- Integrated analog path ---------------------------------------------------------------------
    {
        tape::MachineSpec machine;
        tape::loadMachineSpecFile(std::string(TAPE_PROFILE_DIR) + "/closed_loop_16track_2inch_1970.json", machine, error);
        AnalogPathSettings settings;
        settings.tapeTrackEnabled = false;
        settings.consoleEnabled = true;

        IntegratedAnalogPathEngine path;
        path.prepare(48000.0, 256);
        path.setSettings(settings);
        check(!path.hasConsoleStage() && path.latencySamples() == 0, "integrated path keeps the placeholder console by default");
        check(path.setConsoleStage(spec, {}) && path.hasConsoleStage() && path.latencySamples() == 34 && path.getTelemetry().physicalConsoleStageActive,
              "integrated path runs the physical console stage and reports its latency");

        std::vector<float> l(4800), r(4800), dl, dr;
        for (std::size_t i = 0; i < l.size(); ++i)
            l[i] = r[i] = 0.5f * static_cast<float>(std::sin(2.0 * 3.141592653589793 * 220.0 * i / 48000.0));
        dl = l;
        dr = r;
        for (std::size_t pos = 0; pos < l.size(); pos += 256)
            path.processStereo(l.data() + pos, r.data() + pos, static_cast<int>(std::min<std::size_t>(256, l.size() - pos)));
        console::PhysicalConsoleStage direct;
        direct.prepare(48000.0, 2, spec, {});
        float* ch[2] = { dl.data(), dr.data() };
        direct.process(ch, 2, static_cast<int>(dl.size()));
        double same = 0.0;
        for (std::size_t i = 0; i < l.size(); ++i)
            same = std::max(same, static_cast<double>(std::abs(l[i] - dl[i])));
        check(same < 1e-7, "integrated path output equals the stage run on its own");

        tape::RunSettings rs;
        settings.tapeTrackEnabled = true;
        path.setSettings(settings);
        check(path.setTrackMachine(machine, rs) && path.latencySamples() == 99 + 34, "tape and console latencies add (133 samples)");
        path.clearConsoleStage();
        check(!path.hasConsoleStage() && path.latencySamples() == 99, "the console stage can be cleared, returning to the placeholder");
    }

    // --- Second profile ------------------------------------------------------------------------------
    console::StageSpec american;
    {
        const bool loaded = console::loadStageSpecFile(std::string(CONSOLE_PROFILE_DIR) + "/american_discrete_opamp_line_1970.json", american, error);
        check(loaded && console::validateStageSpec(american).empty() && american.status.rfind("provisional", 0) == 0,
              "American op-amp profile is valid and marked provisional");
        bool allPass = loaded;
        for (const auto& row : console::bench::report(american, 48000.0))
            if (row.hasLimit && !row.pass)
            {
                std::printf("      %s: %s (limit %s)\n", row.name.c_str(), row.measured.c_str(), row.limit.c_str());
                allPass = false;
            }
        check(allPass, "American op-amp model is inside its published figures");
        const auto r = rig(american);
        check(console::bench::thdPercent(r, 1000.0, 20.0, 3) > 2.0 * console::bench::thdPercent(r, 1000.0, 20.0, 2), "its distortion is mostly third harmonic, unlike the class-A stage");
        check(console::bench::thdPercent(r, 1000.0, 20.0) < 0.5 * console::bench::thdPercent(rig(spec), 1000.0, 20.0), "it is cleaner than the class-A stage below the knee");
    }

    // --- Mix bus ---------------------------------------------------------------------------------------
    {
        const double fs = 48000.0, pi = 3.141592653589793;
        const int n = 24000, skip = 9600;
        auto amp = [&](double dbu) { return std::pow(10.0, (dbu - spec.nominalLevelDbu + spec.referenceDbfs) / 20.0); };
        auto sine = [&](double hz, double dbu)
        {
            std::vector<float> x(static_cast<std::size_t>(n));
            for (int i = 0; i < n; ++i)
                x[static_cast<std::size_t>(i)] = static_cast<float>(amp(dbu) * std::sin(2.0 * pi * hz * i / fs));
            return x;
        };
        auto level = [&](const std::vector<float>& y, double hz)
        {
            double s = 0.0, c = 0.0;
            for (int i = skip; i < n; ++i)
            {
                s += y[static_cast<std::size_t>(i)] * std::sin(2.0 * pi * hz * i / fs);
                c += y[static_cast<std::size_t>(i)] * std::cos(2.0 * pi * hz * i / fs);
            }
            return 2.0 * std::sqrt(s * s + c * c) / (n - skip);
        };
        auto mix = [&](const std::vector<std::vector<float>>& stems, const console::BusRunSettings& run = {})
        {
            console::PhysicalConsoleBus bus;
            std::vector<float> l(static_cast<std::size_t>(n)), r(static_cast<std::size_t>(n));
            if (!bus.prepare(fs, static_cast<int>(stems.size()), spec, spec, run))
                return std::vector<float> {};
            std::vector<const float*> ptr;
            for (const auto& s : stems)
            {
                ptr.push_back(s.data());
                ptr.push_back(s.data());
            }
            for (int pos = 0; pos < n; pos += 480)
            {
                std::vector<const float*> block;
                for (const float* p : ptr)
                    block.push_back(p + pos);
                bus.process(block.data(), static_cast<int>(stems.size()), l.data() + pos, r.data() + pos, std::min(480, n - pos));
            }
            return l;
        };
        auto chain = [&](std::vector<float> x)   // one stem through channel stage then bus stage, on its own
        {
            console::PhysicalConsoleStage two;
            console::StageRunSettings run;
            run.stages = 2;
            two.prepare(fs, 1, spec, run);
            float* p = x.data();
            two.process(&p, 1, n);
            return x;
        };

        const auto a = sine(1000.0, 16.0), b = sine(1300.0, 16.0);
        const auto alone = mix({ a }), separate = chain(a);
        double worst = 0.0;
        for (int i = 0; i < n; ++i)
            worst = std::max(worst, static_cast<double>(std::abs(alone[static_cast<std::size_t>(i)] - separate[static_cast<std::size_t>(i)])));
        check(!alone.empty() && worst < 1e-6, "one stem on the bus equals a channel stage followed by a bus stage");
        check(std::abs(20.0 * std::log10(level(alone, 1000.0) / amp(16.0))) < 0.05, "the bus has unity gain for a single stem");

        // Interaction: the bus stage sees the sum, so two stems make a difference tone that
        // separate processing does not.
        const auto together = mix({ a, b });
        auto apart = chain(a);
        const auto apartB = chain(b);
        for (int i = 0; i < n; ++i)
            apart[static_cast<std::size_t>(i)] += apartB[static_cast<std::size_t>(i)];
        const double imBus = level(together, 300.0), imApart = level(apart, 300.0);
        std::printf("      300 Hz difference tone: %.1f dBu on the bus, %.1f dBu processed separately\n",
                    20.0 * std::log10(imBus / amp(0.0)), 20.0 * std::log10(std::max(imApart, 1e-12) / amp(0.0)));
        check(imBus > 4.0 * imApart, "stems intermodulate on the bus, which separate processing cannot do");

        // A loud stem takes headroom from a quiet one.
        const auto quiet = sine(3000.0, -10.0), loud = sine(60.0, 30.0);
        const double quietAlone = level(mix({ quiet }), 3000.0), quietWithLoud = level(mix({ quiet, loud }), 3000.0);
        std::printf("      quiet stem falls %.2f dB when a loud stem overloads the bus\n", 20.0 * std::log10(quietAlone / quietWithLoud));
        check(quietWithLoud < 0.9 * quietAlone, "a loud stem changes how a quiet stem comes through");

        // The same material spread over more stems reaches the bus stage hotter.
        const auto one = mix({ sine(1000.0, 12.0) });
        const auto four = mix({ sine(1000.0, 12.0), sine(1000.0, 12.0), sine(1000.0, 12.0), sine(1000.0, 12.0) });
        const double thdOne = level(one, 3000.0) / level(one, 1000.0), thdFour = level(four, 3000.0) / level(four, 1000.0);
        check(thdFour > 3.0 * thdOne, "four stems summing to +24 dBu distort more than one stem at +12 dBu");

        console::BusRunSettings driven;
        driven.busDriveDb = 10.0;
        const auto hot = mix({ a }, driven);
        check(std::abs(20.0 * std::log10(level(hot, 1000.0) / level(alone, 1000.0))) < 0.3 && level(hot, 3000.0) > 2.0 * level(alone, 3000.0),
              "bus drive adds distortion and is level-compensated");

        // Contract.
        console::PhysicalConsoleBus bus;
        check(!bus.prepare(fs, 0, spec, spec, {}) && !bus.prepare(fs, console::PhysicalConsoleBus::kMaxStems + 1, spec, spec, {}), "zero stems or too many stems are refused");
        auto other = spec;
        other.referenceDbfs = -20.0;
        check(!bus.prepare(fs, 2, spec, other, {}), "channel and bus profiles with different reference levels are refused");
        check(bus.prepare(fs, 2, spec, american, {}) && bus.isReady() && bus.stemCount() == 2, "channel and bus stages may use different profiles");
        std::vector<float> l(256, 1.0f), r(256, 1.0f);
        const float* none[4] = { nullptr, nullptr, nullptr, nullptr };
        bus.process(none, 2, l.data(), r.data(), 256);
        double peak = 0.0;
        for (float v : l)
            peak = std::max(peak, static_cast<double>(std::abs(v)));
        check(peak == 0.0, "missing stems are treated as silence");

        // Latency: impulse-like noise through the bus lines up at the reported delay.
        std::vector<float> burst(static_cast<std::size_t>(n), 0.0f);
        std::uint32_t seed = 5u;
        for (int i = 2000; i < 6000; ++i)
        {
            seed = seed * 1664525u + 1013904223u;
            burst[static_cast<std::size_t>(i)] = 0.01f * (static_cast<float>(seed >> 8) / 8388608.0f - 1.0f);
        }
        for (int i = 2001; i < 6000; ++i)   // soften so the comparison is not dominated by the top octave
            burst[static_cast<std::size_t>(i)] = 0.5f * (burst[static_cast<std::size_t>(i)] + burst[static_cast<std::size_t>(i - 1)]);
        const auto delayed = mix({ burst });
        int best = 0;
        double bestValue = -1e30;
        for (int lag = 0; lag < 200; ++lag)
        {
            double sum = 0.0;
            for (int i = 2000; i < 6000; ++i)
                sum += burst[static_cast<std::size_t>(i)] * delayed[static_cast<std::size_t>(i + lag)];
            if (sum > bestValue)
            {
                bestValue = sum;
                best = lag;
            }
        }
        console::PhysicalConsoleBus sized;
        sized.prepare(fs, 1, spec, spec, {});
        console::PhysicalConsoleBus many;
        many.prepare(fs, 24, spec, spec, {});
        check(best == sized.latencySamples() && many.latencySamples() == sized.latencySamples(),
              "bus latency is " + std::to_string(sized.latencySamples()) + " samples whatever the stem count (measured " + std::to_string(best) + ")");
    }

    std::printf("\n%s: %d failure(s)\n", failures == 0 ? "PASS" : "FAIL", failures);
    return failures == 0 ? 0 : 1;
}
