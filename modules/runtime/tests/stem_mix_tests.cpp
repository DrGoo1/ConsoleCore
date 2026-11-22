// Tests for the stem mix runtime (multitrack tape machine into a console bus).
// Standard library only. These check behaviour and bookkeeping, not sound quality.

#include "PhysicalStemMixRuntime.h"
#include "ConsoleSpecJson.h"
#include "TapeSpecJson.h"

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

    const double kFs = 48000.0, kPi = 3.141592653589793;
    const int kN = 24000;

    std::vector<float> sine(double hz, double amp)
    {
        std::vector<float> x(static_cast<std::size_t>(kN));
        for (int i = 0; i < kN; ++i)
            x[static_cast<std::size_t>(i)] = static_cast<float>(amp * std::sin(2.0 * kPi * hz * i / kFs));
        return x;
    }

    double level(const std::vector<float>& y, double hz)
    {
        double s = 0.0, c = 0.0;
        const int skip = 9600;
        for (int i = skip; i < kN; ++i)
        {
            s += y[static_cast<std::size_t>(i)] * std::sin(2.0 * kPi * hz * i / kFs);
            c += y[static_cast<std::size_t>(i)] * std::cos(2.0 * kPi * hz * i / kFs);
        }
        return 2.0 * std::sqrt(s * s + c * c) / (kN - skip);
    }

    struct Mix
    {
        std::vector<float> left, right;
    };

    Mix run(PhysicalStemMixRuntime& rt, const std::vector<const std::vector<float>*>& stems, int block)
    {
        Mix m { std::vector<float>(static_cast<std::size_t>(kN)), std::vector<float>(static_cast<std::size_t>(kN)) };
        for (int pos = 0; pos < kN; pos += block)
        {
            std::vector<const float*> ptr;
            for (const auto* s : stems)
            {
                ptr.push_back(s != nullptr ? s->data() + pos : nullptr);
                ptr.push_back(s != nullptr ? s->data() + pos : nullptr);
            }
            rt.process(ptr.data(), static_cast<int>(stems.size()), m.left.data() + pos, m.right.data() + pos, std::min(block, kN - pos));
        }
        return m;
    }

    int delayOf(const std::vector<float>& in, const std::vector<float>& out)
    {
        int best = 0;
        double bestValue = -1e30;
        for (int lag = 0; lag < 400; ++lag)
        {
            double sum = 0.0;
            for (int i = 2000; i < 6000; ++i)
                sum += in[static_cast<std::size_t>(i)] * out[static_cast<std::size_t>(i + lag)];
            if (sum > bestValue)
            {
                bestValue = sum;
                best = lag;
            }
        }
        return best;
    }
}

int main()
{
    std::string error;
    StemMixConfig base;
    if (!tape::loadMachineSpecFile(std::string(TAPE_PROFILE_DIR) + "/closed_loop_16track_2inch_1970.json", base.tapeMachine, error)
        || !console::loadStageSpecFile(std::string(CONSOLE_PROFILE_DIR) + "/british_class_a_discrete_line_1970.json", base.channelStage, error))
    {
        std::printf("FAIL  cannot load profiles: %s\n", error.c_str());
        return 1;
    }
    base.busStage = base.channelStage;
    base.tapeRun.flutterEnabled = false;   // keeps the comparisons below exact

    std::vector<float> burst(static_cast<std::size_t>(kN), 0.0f);
    std::uint32_t seed = 9u;
    float previous = 0.0f;
    for (int i = 2000; i < 6000; ++i)
    {
        seed = seed * 1664525u + 1013904223u;
        const float v = 0.01f * (static_cast<float>(seed >> 8) / 8388608.0f - 1.0f);
        burst[static_cast<std::size_t>(i)] = 0.5f * (v + previous);
        previous = v;
    }

    // --- Console only -----------------------------------------------------------------------
    {
        StemMixConfig cfg = base;
        cfg.numStems = 3;
        PhysicalStemMixRuntime rt;
        check(rt.prepare(kFs, 256, cfg) && rt.isReady(), "prepares with the console bus alone");
        const auto a = sine(440.0, 0.1), b = sine(660.0, 0.05);
        const Mix m = run(rt, { &a, &b, nullptr }, 256);

        console::PhysicalConsoleBus direct;
        direct.prepare(kFs, 3, cfg.channelStage, cfg.busStage, cfg.busRun);
        std::vector<float> l(static_cast<std::size_t>(kN)), r(static_cast<std::size_t>(kN));
        const float* ptr[6] = { a.data(), a.data(), b.data(), b.data(), nullptr, nullptr };
        direct.process(ptr, 3, l.data(), r.data(), kN);
        double worst = 0.0;
        for (int i = 0; i < kN; ++i)
            worst = std::max(worst, static_cast<double>(std::abs(l[static_cast<std::size_t>(i)] - m.left[static_cast<std::size_t>(i)])));
        check(worst < 1e-7, "without tape the runtime equals the console bus on its own");
        check(rt.latencySamples() == direct.latencySamples(), "and reports the bus latency (" + std::to_string(rt.latencySamples()) + " samples)");
        check(rt.telemetry().activeStemCount == 2 && rt.stemStates().size() == 3 && rt.stemStates()[0].peak > 0.09f && rt.stemStates()[2].peak == 0.0f,
              "meters report two active stems and their input peaks");
        check(a[100] == static_cast<float>(0.1 * std::sin(2.0 * kPi * 440.0 * 100 / kFs)), "input buffers are left untouched");
    }

    // --- Tape into console -----------------------------------------------------------------------
    {
        StemMixConfig cfg = base;
        cfg.numStems = 2;
        cfg.tapeEnabled = true;
        PhysicalStemMixRuntime rt;
        check(rt.prepare(kFs, 256, cfg), "prepares with every stem on tape");
        const int measured = delayOf(burst, run(rt, { &burst, nullptr }, 256).left);
        check(std::abs(measured - rt.latencySamples()) <= 1, "reported latency (" + std::to_string(rt.latencySamples()) + " samples) is within one sample of measured (" + std::to_string(measured) + ")");

        // One stem on tape, one skipping it: both must arrive together.
        cfg.stemUsesTape = { true, false };
        PhysicalStemMixRuntime mixed;
        check(mixed.prepare(kFs, 256, cfg), "prepares with one stem on tape and one skipping it");
        const int onTape = delayOf(burst, run(mixed, { &burst, nullptr }, 256).left);
        mixed.reset();
        const int offTape = delayOf(burst, run(mixed, { nullptr, &burst }, 256).left);
        check(std::abs(onTape - offTape) <= 1, "a stem that skips the tape stays aligned with one that does not (" + std::to_string(onTape) + " and " + std::to_string(offTape) + " samples)");

        // The tape changes the stem that uses it and leaves the other clean.
        const auto hot = sine(1000.0, 0.5);
        mixed.reset();
        const double tapedThird = level(run(mixed, { &hot, nullptr }, 256).left, 3000.0);
        mixed.reset();
        const double cleanThird = level(run(mixed, { nullptr, &hot }, 256).left, 3000.0);
        check(tapedThird > 5.0 * cleanThird, "tape distortion appears only on the stem assigned to tape");

        // Block size must not matter, and reset must restore the starting state.
        mixed.reset();
        const Mix small = run(mixed, { &hot, &burst }, 64);
        mixed.reset();
        const Mix large = run(mixed, { &hot, &burst }, 1000);   // larger than the prepared block
        double worst = 0.0;
        for (int i = 0; i < kN; ++i)
            worst = std::max(worst, static_cast<double>(std::abs(small.left[static_cast<std::size_t>(i)] - large.left[static_cast<std::size_t>(i)])));
        check(worst < 1e-6, "output is the same for any block size, including blocks larger than prepared");
    }

    // --- Shared transport ---------------------------------------------------------------------------
    {
        StemMixConfig cfg = base;
        cfg.numStems = 2;
        cfg.tapeEnabled = true;
        cfg.tapeRun.flutterEnabled = true;
        PhysicalStemMixRuntime rt;
        rt.prepare(kFs, 256, cfg);
        const auto tone = sine(3150.0, 0.05);
        const Mix first = run(rt, { &tone, nullptr }, 256);
        rt.reset();
        const Mix second = run(rt, { nullptr, &tone }, 256);
        double worst = 0.0;
        for (int i = 0; i < kN; ++i)
            worst = std::max(worst, static_cast<double>(std::abs(first.left[static_cast<std::size_t>(i)] - second.left[static_cast<std::size_t>(i)])));
        check(worst < 1e-6, "every stem rides the same transport: flutter is identical on different tracks");
    }

    // --- Limits ---------------------------------------------------------------------------------------
    {
        StemMixConfig cfg = base;
        cfg.tapeEnabled = true;
        PhysicalStemMixRuntime rt;
        cfg.numStems = 8;
        check(rt.prepare(kFs, 128, cfg), "eight stereo stems fit a 16-track machine");
        cfg.numStems = 9;
        check(!rt.prepare(kFs, 128, cfg) && !rt.isReady() && std::string(rt.lastError()).find("tracks") != std::string::npos, "nine stereo stems do not, and the reason is given");
        cfg.stemUsesTape.assign(9, true);
        cfg.stemUsesTape[8] = false;
        check(rt.prepare(kFs, 128, cfg), "unless one of them skips the tape");
        cfg = base;
        cfg.tapeEnabled = true;
        cfg.tapeRun.ips = 3.75;
        check(!rt.prepare(kFs, 128, cfg) && std::string(rt.lastError()).find("speed") != std::string::npos, "an unsupported tape speed is refused with a reason");
        cfg = base;
        cfg.numStems = 0;
        check(!rt.prepare(kFs, 128, cfg), "zero stems are refused");

        std::vector<float> l(64, 0.5f), r(64, 0.5f);
        rt.process(nullptr, 0, l.data(), r.data(), 64);
        check(l[10] == 0.5f, "an unprepared runtime leaves the output alone");

        cfg = base;
        cfg.numStems = 2;
        cfg.tapeEnabled = true;
        rt.prepare(kFs, 128, cfg);
        std::vector<float> nasty(static_cast<std::size_t>(kN), 0.5f);
        nasty[100] = std::nanf("");
        nasty[200] = INFINITY;
        nasty[300] = 1.0e6f;
        const Mix m = run(rt, { &nasty, &nasty }, 128);
        bool finite = true;
        double biggest = 0.0;
        for (float v : m.left)
        {
            finite = finite && std::isfinite(v);
            biggest = std::max(biggest, static_cast<double>(std::abs(v)));
        }
        check(finite && biggest < 8.0, "extreme and non-finite input stays finite and bounded");
    }

    std::printf("\n%s: %d failure(s)\n", failures == 0 ? "PASS" : "FAIL", failures);
    return failures == 0 ? 0 : 1;
}
