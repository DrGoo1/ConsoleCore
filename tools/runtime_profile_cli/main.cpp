#include "../../modules/runtime/RuntimeSessionController.h"
#include "../../modules/routing/LoopbackRoutingProfile.h"
#include "../../modules/routing/LoopbackRoutingProfileIO.h"
#include "../../modules/latency/LatencyCalibration.h"
#include "../../modules/latency/LatencyCalibrationIO.h"
#include <iostream>

int main()
{
    RuntimeSessionController runtime;

    RuntimeSnapshot clean;
    clean.name = "Clean Console";
    clean.drive = 0.12f;

    RuntimeSnapshot driven;
    driven.name = "Driven Mix";
    driven.drive = 0.34f;
    driven.railStressAmount = 0.48f;

    runtime.addSnapshot(clean);
    runtime.addSnapshot(driven);

    const bool recalled = runtime.recallSnapshot("Driven Mix");
    auto current = runtime.getCurrentSnapshot();

    std::cout << "ConsoleCore Runtime Profile CLI\n";
    std::cout << "Recall Driven Mix: " << (recalled ? "yes" : "no") << "\n";
    if (current.has_value())
        std::cout << "Current Snapshot: " << current->name << " drive=" << current->drive << "\n";

    LoopbackRoutingProfile profile;
    profile.profileName = "CLI Test Profile";
    profile.inputDeviceName = "ConsoleCore Sends";
    profile.outputDeviceName = "ConsoleCore Returns";
    LoopbackRoutingProfileIO::save(profile, "runtime_profile_test.json");

    LatencyCalibration latency;
    latency.profileName = "CLI Test Profile";
    latency.dawName = "LUNA";
    latency.measuredRoundtripSamples = 1024;
    LatencyCalibrationIO::save(latency, "latency_test.json");

    std::cout << "Wrote runtime_profile_test.json and latency_test.json\n";
    return 0;
}