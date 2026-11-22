#pragma once
// Measurements of a PhysicalTapeMachine, for tests and the bench tool.
// Offline only: allocates freely.

#include "TapeMachineSpec.h"
#include <string>
#include <vector>

namespace tape::bench
{
    struct Rig
    {
        MachineSpec spec;
        RunSettings run;
        double sampleRate = 48000.0;
    };

    // Runs one channel through a freshly prepared machine. Empty result if the speed is not in the spec.
    std::vector<float> runMono(const Rig& rig, const std::vector<float>& input);

    double responseDb(const Rig& rig, double hz, double levelDb = -20.0);  // gain re unity, flutter off
    double hd3Percent(const Rig& rig, double hz, double levelDb);          // flutter off
    double flutterRmsPercent(const Rig& rig, double loHz, double hiHz, double seconds = 6.0);
    double noiseDb(const Rig& rig);                                        // 20 Hz - 20 kHz, re operating level
    int measuredLatency(const Rig& rig);

    struct Row
    {
        std::string name;
        std::string measured;
        std::string limit;     // empty when nothing is published
        std::string basis;     // published / derived / assumed
        bool hasLimit = false;
        bool pass = true;
    };

    // Compares the model at one speed with whatever the profile says was published.
    std::vector<Row> report(const MachineSpec& spec, double ips, double sampleRate);
}
