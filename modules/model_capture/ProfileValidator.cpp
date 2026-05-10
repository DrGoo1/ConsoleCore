#include "ProfileValidator.h"
#include "../analog_physics/AnalogPhysicsCore.h"
#include <vector>
#include <cmath>
#include <algorithm>

ProfileValidationReport ProfileValidator::validate(const AnalogPhysicsParameters& params)
{
    constexpr double fs = 48000.0;
    constexpr int n = 4096;

    std::vector<float> l(n), r(n);
    for (int i = 0; i < n; ++i)
    {
        l[i] = 0.35f * std::sin(2.0 * 3.141592653589793 * 997.0 * i / fs);
        r[i] = 0.25f * std::sin(2.0 * 3.141592653589793 * 1203.0 * i / fs);
        if (i == 256) l[i] += 0.8f;
    }

    AnalogPhysicsCore core;
    core.prepare(fs, n);
    core.setParameters(params);
    core.processBlock(l.data(), r.data(), n, 8);

    ProfileValidationReport report;
    report.stable = true;

    for (int i = 0; i < n; ++i)
    {
        if (!std::isfinite(l[i]) || !std::isfinite(r[i]) || std::abs(l[i]) > 4.0f || std::abs(r[i]) > 4.0f)
            report.stable = false;
    }

    const auto& t = core.getTelemetry();
    report.peakOutput = t.peakOutput;
    report.railStress = t.railStress;
    report.headroomGain = t.headroomGain;
    report.message = report.stable ? "PASS: profile stable" : "FAIL: profile unstable";
    return report;
}