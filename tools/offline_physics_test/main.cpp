#include "../../modules/analog_physics/AnalogPhysicsCore.h"
#include <cmath>
#include <iostream>
#include <vector>
#include <limits>

static bool isStable(const std::vector<float>& a, const std::vector<float>& b)
{
    for (size_t i = 0; i < a.size(); ++i)
    {
        if (!std::isfinite(a[i]) || !std::isfinite(b[i]))
            return false;
        if (std::abs(a[i]) > 4.0f || std::abs(b[i]) > 4.0f)
            return false;
    }
    return true;
}

int main()
{
    constexpr double fs = 48000.0;
    constexpr int n = 4096;

    std::vector<float> l(n), r(n);

    // sine + impulse + deterministic pseudo-noise tail
    for (int i = 0; i < n; ++i)
    {
        const float sine = 0.25f * std::sin(2.0 * M_PI * 1000.0 * static_cast<double>(i) / fs);
        const float impulse = (i == 128) ? 0.9f : 0.0f;
        const float noise = 0.02f * std::sin(2.0 * M_PI * 173.0 * i / fs) * std::sin(2.0 * M_PI * 19.0 * i / fs);
        l[i] = sine + impulse + noise;
        r[i] = 0.8f * sine - 0.5f * impulse + noise;
    }

    AnalogPhysicsCore core;
    core.prepare(fs, n);

    AnalogPhysicsParameters params;
    params.drive = 0.32f;
    params.railStressAmount = 0.50f;
    params.transformerWeight = 0.40f;
    params.crosstalkAmount = 0.10f;
    params.tapeMemoryAmount = 0.20f;
    core.setParameters(params);

    core.processBlock(l.data(), r.data(), n, 8);

    const auto& t = core.getTelemetry();

    std::cout << "ConsoleCore Offline Physics Test\n";
    std::cout << "Rail Voltage: " << t.railVoltage << "\n";
    std::cout << "Rail Stress: " << t.railStress << "\n";
    std::cout << "Temporal Memory: " << t.memory << "\n";
    std::cout << "Headroom Gain: " << t.headroomGain << "\n";
    std::cout << "Peak Output: " << t.peakOutput << "\n";
    std::cout << "Spectral Load L/M/H/B: "
              << t.spectralLoad.low << " / "
              << t.spectralLoad.mid << " / "
              << t.spectralLoad.high << " / "
              << t.spectralLoad.broadband << "\n";

    const bool ok = isStable(l, r);
    std::cout << (ok ? "PASS: stable output\n" : "FAIL: unstable output\n");
    return ok ? 0 : 1;
}