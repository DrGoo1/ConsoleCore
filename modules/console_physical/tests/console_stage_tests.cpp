// Tests for the physical console stage model. Standard library only.
//
// These check that the model is well behaved and sits inside figures taken from
// documentation. They cannot show that it sounds like any real console.

#include "ConsoleBench.h"
#include "ConsoleSpecJson.h"
#include "PhysicalConsoleStage.h"
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
        check(console::bench::measuredLatency(rig(spec)) == console::PhysicalConsoleStage::latencySamples(),
              "reported latency matches measured latency (" + std::to_string(console::PhysicalConsoleStage::latencySamples()) + " samples)");

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

    std::printf("\n%s: %d failure(s)\n", failures == 0 ? "PASS" : "FAIL", failures);
    return failures == 0 ? 0 : 1;
}
