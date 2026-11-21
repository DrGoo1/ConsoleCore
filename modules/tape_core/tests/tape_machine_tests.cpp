// Tests for the physical tape machine model. Standard library only.
//
// These check that the model is well behaved and sits inside figures taken from
// documentation. They cannot show that it sounds like any real machine.

#include "PhysicalTapeMachine.h"
#include "TapeBench.h"
#include "TapeSpecJson.h"
#include "TapeTrackProcessor.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

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

    tape::MachineSpec load(const char* name)
    {
        tape::MachineSpec spec;
        std::string error;
        const std::string path = std::string(TAPE_PROFILE_DIR) + "/" + name;
        if (!tape::loadMachineSpecFile(path, spec, error))
        {
            std::printf("FAIL  cannot load %s: %s\n", path.c_str(), error.c_str());
            ++failures;
        }
        return spec;
    }

    tape::bench::Rig rig(const tape::MachineSpec& spec, double ips, double fs = 48000.0)
    {
        tape::bench::Rig r;
        r.spec = spec;
        r.sampleRate = fs;
        r.run.ips = ips;
        return r;
    }

    std::string label(const char* text, double ips) { return std::string(text) + " at " + std::to_string(ips).substr(0, 4) + " ips"; }
}

int main()
{
    const tape::MachineSpec closedLoop = load("closed_loop_16track_2inch_1970.json");
    const tape::MachineSpec servo = load("transformerless_servo_24track_2inch_1979.json");
    if (failures > 0)
        return 1;

    // --- Profiles ---------------------------------------------------------------------
    for (const auto* spec : { &closedLoop, &servo })
    {
        const auto problems = tape::validateMachineSpec(*spec);
        for (const auto& p : problems)
            std::printf("      %s\n", p.c_str());
        check(problems.empty(), spec->id + ": profile is valid and every sound-shaping field has evidence");
        check(spec->status.find("not validated") != std::string::npos, spec->id + ": status says it is not validated");
        check(spec->referenceMachine.find("not a claim of equivalence") != std::string::npos, spec->id + ": reference text disclaims equivalence");
    }
    check(servo.status.rfind("provisional", 0) == 0, "servo profile is marked provisional until its manual is read");
    {
        tape::MachineSpec out;
        std::string error;
        check(!tape::parseMachineSpec("{\"schema\":\"something.else\"}", out, error) && !error.empty(), "wrong schema is rejected with a message");
        check(!tape::parseMachineSpec("{ not json", out, error), "malformed JSON is rejected");
        tape::MachineSpec gap = closedLoop;
        gap.evidence.erase("tape.coatingThicknessMicrons");
        check(!tape::validateMachineSpec(gap).empty(), "a field without evidence fails validation");
        tape::MachineSpec silly = closedLoop;
        silly.hd3PercentAtCal = 60.0;
        check(!tape::validateMachineSpec(silly).empty(), "an implausible value fails validation");
    }

    // --- Published figures, closed-loop machine -----------------------------------------
    for (double ips : { 7.5, 15.0, 30.0 })
    {
        bool allPass = true;
        int limits = 0;
        for (const auto& row : tape::bench::report(closedLoop, ips, 48000.0))
            if (row.hasLimit)
            {
                ++limits;
                if (!row.pass)
                {
                    std::printf("      %s: %s (limit %s)\n", row.name.c_str(), row.measured.c_str(), row.limit.c_str());
                    allPass = false;
                }
            }
        check(allPass && limits >= 3, label("closed-loop model is inside every published figure", ips));
    }
    for (double fs : { 44100.0, 96000.0 })
    {
        auto r = rig(closedLoop, 15.0, fs);
        const double ref = tape::bench::responseDb(r, 700.0);
        const double hi = tape::bench::responseDb(r, 15000.0) - ref, lo = tape::bench::responseDb(r, 100.0) - ref;
        check(std::abs(ref) < 0.5 && std::abs(hi) <= 2.0 && std::abs(lo) <= 2.0, "response holds at " + std::to_string(static_cast<int>(fs)) + " Hz sample rate");
    }

    // --- Physical behaviour ---------------------------------------------------------------
    {
        auto r = rig(closedLoop, 15.0);
        r.run.linearizerEnabled = false;
        const double h0 = tape::bench::hd3Percent(r, 1000.0, 0.0), h6 = tape::bench::hd3Percent(r, 1000.0, 6.0), h12 = tape::bench::hd3Percent(r, 1000.0, 12.0);
        check(h0 < h6 && h6 < h12, "distortion rises with level");
        check(tape::bench::responseDb(r, 1000.0, 12.0) < tape::bench::responseDb(r, 1000.0, -20.0) - 1.0, "tape compresses when driven hard");
        check(tape::bench::hd3Percent(r, 1000.0, -20.0) < 0.05, "tape is clean at low level");
        r.run.linearizerEnabled = true;
        check(tape::bench::hd3Percent(r, 1000.0, 6.0) < h6 * 0.4, "linearizer cuts third harmonic at the calibration level");

        // Level-dependent treble loss is strongest at the slowest speed.
        auto slow = rig(closedLoop, 7.5), fast = rig(closedLoop, 30.0);
        slow.run.linearizerEnabled = fast.run.linearizerEnabled = false;
        const double slowSquash = tape::bench::responseDb(slow, 8000.0, 9.0) - tape::bench::responseDb(slow, 8000.0, -20.0);
        const double fastSquash = tape::bench::responseDb(fast, 8000.0, 9.0) - tape::bench::responseDb(fast, 8000.0, -20.0);
        check(slowSquash < fastSquash - 0.5, "treble compresses more at 7.5 ips than at 30 ips");
    }
    {
        // The head bump sits at a fixed wavelength, so it doubles in frequency with tape speed.
        tape::PhysicalTapeMachine a, b;
        tape::RunSettings ra, rb;
        ra.ips = 15.0;
        rb.ips = 30.0;
        a.prepare(48000.0, 256, 1, closedLoop, ra);
        b.prepare(48000.0, 256, 1, closedLoop, rb);
        check(std::abs(b.headBumpHz() / a.headBumpHz() - 2.0) < 1e-6, "head bump frequency doubles from 15 to 30 ips");
        auto r15 = rig(closedLoop, 15.0), r30 = rig(closedLoop, 30.0);
        const double bump15 = tape::bench::responseDb(r15, a.headBumpHz()) - tape::bench::responseDb(r15, 700.0);
        const double bump30 = tape::bench::responseDb(r30, b.headBumpHz()) - tape::bench::responseDb(r30, 700.0);
        check(bump15 > 0.3 && bump30 > 0.3, "response rises at the head bump at both speeds");
        check(tape::bench::responseDb(r30, 30.0) < tape::bench::responseDb(r15, 30.0) - 1.5, "30 ips has less bass extension than 15 ips");
    }
    {
        // Transformers distort most at low frequency and high level; the transformerless machine has none.
        tape::MachineSpec xfmrOnly = closedLoop;
        xfmrOnly.hd3PercentAtCal = 0.1;    // make the tape nearly linear up to maximum output,
        xfmrOnly.hd3CalLevelDb = xfmrOnly.maxOutputDb;  // so the transformer can be seen on its own
        xfmrOnly.hasLinearizer = false;
        xfmrOnly.inputTransformer = false;
        xfmrOnly.speeds[1].lfTimeConstantUs = 0.0;
        auto r = rig(xfmrOnly, 15.0);
        const double at30 = tape::bench::hd3Percent(r, 30.0, xfmrOnly.maxOutputDb - 0.5);
        const double at200 = tape::bench::hd3Percent(r, 200.0, xfmrOnly.maxOutputDb - 0.5);
        tape::MachineSpec none = xfmrOnly;
        none.outputTransformer = false;
        const double without = tape::bench::hd3Percent(rig(none, 15.0), 30.0, xfmrOnly.maxOutputDb - 0.5);
        std::printf("      output transformer at 30 Hz near maximum output: %.2f %%, at 200 Hz: %.2f %%, removed: %.2f %%\n", at30, at200, without);
        check(at30 > without + 0.3 && at30 < without + 1.0, "output transformer adds under 1 percent third harmonic at 30 Hz at maximum output (published bound)");
        check(at200 < at30, "transformer distortion falls as frequency rises");
        check(!servo.inputTransformer && !servo.outputTransformer, "servo profile has no transformers");
    }

    // --- Transport --------------------------------------------------------------------------
    {
        auto r = rig(closedLoop, 15.0);
        const double on = tape::bench::flutterRmsPercent(r, 0.5, 300.0);
        r.run.flutterEnabled = false;
        const double off = tape::bench::flutterRmsPercent(r, 0.5, 300.0);
        std::printf("      flutter on %.4f %%, off %.5f %%\n", on, off);
        check(on > 0.01 && on <= 0.04, "flutter is present and inside the published 0.04 percent limit at 15 ips");
        check(off < 0.002, "flutter can be switched off");

        // One transport: every channel sees the same speed deviation.
        tape::PhysicalTapeMachine m;
        tape::RunSettings run;
        m.prepare(48000.0, 512, 2, closedLoop, run);
        std::vector<float> left(24000), right(24000);
        for (std::size_t i = 0; i < left.size(); ++i)
            left[i] = right[i] = 0.05f * static_cast<float>(std::sin(2.0 * 3.141592653589793 * 3150.0 * i / 48000.0));
        for (std::size_t pos = 0; pos < left.size(); pos += 512)
        {
            float* ch[2] = { left.data() + pos, right.data() + pos };
            m.process(ch, 2, static_cast<int>(std::min<std::size_t>(512, left.size() - pos)));
        }
        double diff = 0.0;
        for (std::size_t i = 0; i < left.size(); ++i)
            diff = std::max(diff, static_cast<double>(std::abs(left[i] - right[i])));
        check(diff < 1e-6, "channels of one machine share the same flutter");
    }

    // --- Engine contract ----------------------------------------------------------------------
    {
        auto r = rig(closedLoop, 15.0);
        tape::PhysicalTapeMachine m;
        m.prepare(48000.0, 512, 1, closedLoop, r.run);
        check(tape::bench::measuredLatency(r) == m.latencySamples(), "reported latency matches measured latency (" + std::to_string(m.latencySamples()) + " samples)");

        std::vector<float> silence(4096, 0.0f);
        const auto quiet = tape::bench::runMono(r, silence);
        double peak = 0.0;
        for (float v : quiet)
            peak = std::max(peak, static_cast<double>(std::abs(v)));
        check(peak == 0.0, "silence in gives silence out when noise is off");

        r.run.noiseEnabled = true;
        const double noise = tape::bench::noiseDb(r);
        check(std::abs(noise + closedLoop.findSpeed(15.0)->snrDb) < 2.0, "noise, when enabled, lands within 2 dB of the profile's figure");

        std::vector<float> nasty(4096);
        for (std::size_t i = 0; i < nasty.size(); ++i)
            nasty[i] = (i % 97 == 0) ? 1.0e6f : ((i % 31 == 0) ? -50.0f : 0.5f);
        nasty[100] = std::nanf("");
        nasty[200] = INFINITY;
        const auto survived = tape::bench::runMono(rig(closedLoop, 15.0), nasty);
        bool finite = true;
        double biggest = 0.0;
        for (float v : survived)
        {
            finite = finite && std::isfinite(v);
            biggest = std::max(biggest, static_cast<double>(std::abs(v)));
        }
        check(finite && biggest < 8.0, "extreme and non-finite input stays finite and bounded");

        // Same input twice after reset gives the same output, and block size does not matter.
        std::vector<float> a(3000), b, c;
        for (std::size_t i = 0; i < a.size(); ++i)
            a[i] = 0.2f * static_cast<float>(std::sin(0.05 * i) + 0.5 * std::sin(0.31 * i));
        b = c = a;
        tape::PhysicalTapeMachine x;
        tape::RunSettings run;
        x.prepare(48000.0, 512, 1, closedLoop, run);
        float* pb = b.data();
        x.process(&pb, 1, static_cast<int>(b.size()));   // longer than the prepared block: handled in pieces
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

        tape::RunSettings missing;
        missing.ips = 3.75;
        tape::PhysicalTapeMachine y;
        check(!y.prepare(48000.0, 512, 1, closedLoop, missing) && !y.isReady(), "an unsupported speed is refused, not approximated");
        std::vector<float> untouched(64, 0.25f);
        float* pu = untouched.data();
        y.process(&pu, 1, 64);
        check(untouched[10] == 0.25f, "an unprepared machine passes audio through unchanged");
    }

    // --- Existing generic processor still works ---------------------------------------------------
    {
        TapeTrackProcessor legacy;
        TapeMachineProfile profile;
        TapeSharedState state;
        legacy.prepare(48000.0, 256);
        std::vector<float> l(256, 0.1f), rr(256, 0.1f);
        legacy.processStereo(l.data(), rr.data(), 256, profile, state);
        check(std::isfinite(l[100]) && std::isfinite(rr[100]), "the original generic tape processor is unchanged and runs");
    }

    std::printf("\n%s: %d failure(s)\n", failures == 0 ? "PASS" : "FAIL", failures);
    return failures == 0 ? 0 : 1;
}
