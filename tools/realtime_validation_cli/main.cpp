#include "../../modules/runtime/PerStemTapeRuntime.h"
#include "../../modules/runtime/StemBusManager.h"
#include <iostream>
#include <vector>
#include <cmath>

static void fill(std::vector<float>& x, double fs, double freq)
{
    for (size_t i = 0; i < x.size(); ++i)
        x[i] = 0.25f * std::sin(2.0 * 3.141592653589793 * freq * i / fs);
}

int main()
{
    constexpr int stems = 8;
    constexpr int n = 1024;
    constexpr double fs = 48000.0;

    PerStemTapeRuntime runtime;
    runtime.prepare(fs, n, stems);

    StemBusManager buses;
    buses.resize(stems);

    TapeMachineProfile profile;
    profile.saturationAmount = 0.35f;

    for (int s = 0; s < stems; ++s)
    {
        std::vector<float> l(n), r(n);

        fill(l, fs, 100.0 + s * 70.0);
        fill(r, fs, 150.0 + s * 80.0);

        runtime.processStem(
            s,
            l.data(),
            r.data(),
            n,
            profile
        );

        float peak = 0.0f;
        for (int i = 0; i < n; ++i)
            peak = std::max(peak, std::max(std::abs(l[i]), std::abs(r[i])));

        buses.stem(s).peak = peak;
    }

    const auto& state = runtime.sharedState();

    std::cout << "ConsoleCore Tape Integration Milestone 2 CLI\n";
    std::cout << "Tape Compression: " << state.tapeCompression << "\n";
    std::cout << "Low Frequency Saturation: " << state.lowFrequencySaturation << "\n";
    std::cout << "Bias Stress: " << state.tapeBiasStress << "\n";
    std::cout << "Stem Count: " << buses.stems().size() << "\n";

    return 0;
}
