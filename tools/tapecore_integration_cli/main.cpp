#include "../../modules/integration/IntegratedAnalogPathEngine.h"
#include <iostream>
#include <vector>
#include <cmath>

static bool stable(const std::vector<float>& l, const std::vector<float>& r)
{
    for (size_t i = 0; i < l.size(); ++i)
    {
        if (!std::isfinite(l[i]) || !std::isfinite(r[i]))
            return false;
        if (std::abs(l[i]) > 4.0f || std::abs(r[i]) > 4.0f)
            return false;
    }
    return true;
}

static void fillTestSignal(std::vector<float>& l, std::vector<float>& r, double fs)
{
    for (size_t i = 0; i < l.size(); ++i)
    {
        const double t = static_cast<double>(i) / fs;
        l[i] = 0.25f * std::sin(2.0 * 3.141592653589793 * 1000.0 * t);
        r[i] = 0.20f * std::sin(2.0 * 3.141592653589793 * 1200.0 * t);
        if (i == 128) l[i] += 0.8f;
        if (i == 256) r[i] -= 0.6f;
    }
}

int main()
{
    constexpr double fs = 48000.0;
    constexpr int n = 4096;

    IntegratedAnalogPathEngine engine;
    engine.prepare(fs, n);

    AnalogPathSettings settings;
    settings.activeStemCount = 8;
    settings.consoleEnabled = true;

    for (int test = 0; test < 3; ++test)
    {
        std::vector<float> l(n), r(n);
        fillTestSignal(l, r, fs);

        settings.tapeTrackEnabled = test >= 1;
        settings.printTapeEnabled = test >= 2;

        settings.trackTapeProfile.name = "Generic 2-inch 24-track";
        settings.trackTapeProfile.tapeSpeedIps = 15.0f;
        settings.trackTapeProfile.saturationAmount = 0.38f;
        settings.trackTapeProfile.headBumpAmount = 0.30f;
        settings.trackTapeProfile.hysteresisAmount = 0.34f;

        settings.printTapeProfile.name = "Generic 1/2-inch 2-track print deck";
        settings.printTapeProfile.tapeSpeedIps = 30.0f;
        settings.printTapeProfile.saturationAmount = 0.30f;
        settings.printTapeProfile.headBumpAmount = 0.16f;
        settings.printTapeProfile.hysteresisAmount = 0.28f;

        engine.setSettings(settings);
        engine.reset();
        engine.processStereo(l.data(), r.data(), n);

        if (!stable(l, r))
        {
            std::cout << "FAIL: integrated path unstable on test " << test << "\n";
            return 1;
        }
    }

    const auto telemetry = engine.getTelemetry();

    std::cout << "ConsoleCore + TapeCore Integration CLI\n";
    std::cout << "PASS: all integrated analog paths stable\n";
    std::cout << "Peak: " << telemetry.peak << "\n";
    std::cout << "Tape Bias Stress: " << telemetry.tapeBiasStress << "\n";
    std::cout << "Tape Compression: " << telemetry.tapeCompression << "\n";
    std::cout << "Low Frequency Saturation: " << telemetry.lowFrequencySaturation << "\n";
    std::cout << "Print Tape Stress: " << telemetry.printTapeStress << "\n";
    return 0;
}
