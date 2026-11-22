#pragma once
// Measurements of a console stage model, set beside whatever its profile records as published.

#include "ConsoleStageSpec.h"
#include <string>
#include <vector>

namespace console::bench
{
    struct Rig
    {
        StageSpec spec;
        StageRunSettings run;
        double sampleRate = 48000.0;
    };

    struct Row
    {
        std::string name, measured, limit, basis;
        bool hasLimit = false, pass = true;
    };

    std::vector<float> runMono(const Rig& rig, const std::vector<float>& input);
    double responseDb(const Rig& rig, double hz, double levelDbu);
    // Harmonics 2..5 that fall below half the sample rate; `only` picks one harmonic (0 = all).
    double thdPercent(const Rig& rig, double hz, double levelDbu, int only = 0);
    double noiseDbu(const Rig& rig);
    int measuredLatency(const Rig& rig);

    // One stage, as the published figures describe one module.
    std::vector<Row> report(const StageSpec& spec, double sampleRate);
}
