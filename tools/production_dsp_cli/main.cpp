#include "../../modules/integration/ProductionDspEngine.h"
#include <iostream>
#include <vector>
#include <cmath>

int main()
{
    constexpr int n = 1024;
    constexpr double fs = 48000.0;

    std::vector<float> l(n), r(n);
    for (int i = 0; i < n; ++i)
    {
        l[i] = 0.25f * std::sin(2.0 * 3.141592653589793 * 1000.0 * i / fs);
        r[i] = 0.25f * std::sin(2.0 * 3.141592653589793 * 1200.0 * i / fs);
    }

    ProductionDspEngine engine;
    engine.prepare(fs, n);

    ProductionDspSettings settings;
    settings.activeStemCount = 8;
    engine.setSettings(settings);
    engine.processStereo(l.data(), r.data(), n);

    auto t = engine.getTelemetry();

    std::cout << "ConsoleCore Production DSP CLI\n";
    std::cout << "Peak: " << t.peak << "\n";
    std::cout << "Rail Stress: " << t.railStress << "\n";
    return 0;
}