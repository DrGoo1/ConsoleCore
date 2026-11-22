#pragma once
// The session signal path built from the documentation-derived models: stereo stems are
// recorded to one multitrack tape machine (optional, and per stem), then mixed on a console
// bus to stereo.
//
//   stems -> tape machine (two tracks per stem, one shared transport) -> console bus -> stereo
//
// One machine carries every stem, so all tracks share the same wow and flutter, and the
// machine's track count limits how many stems can be recorded. Stems that skip the tape are
// delayed by the machine's latency so everything stays aligned.
//
// Not validated against hardware. Real-time contract: prepare() allocates and calibrates
// and must not be called on the audio thread; process() does not allocate, lock or throw.

#include "IntegratedRealtimeTelemetry.h"
#include "StemBusManager.h"
#include "../console_physical/PhysicalConsoleBus.h"
#include "../tape_core/PhysicalTapeMachine.h"

#include <vector>

struct StemMixConfig
{
    int numStems = 2;

    bool tapeEnabled = false;
    tape::MachineSpec tapeMachine;
    tape::RunSettings tapeRun;
    std::vector<bool> stemUsesTape;   // empty = every stem; otherwise one entry per stem

    console::StageSpec channelStage;
    console::StageSpec busStage;
    console::BusRunSettings busRun;
};

class PhysicalStemMixRuntime
{
public:
    // False, with a reason from lastError(), if the configuration cannot be run: for example
    // more stems than the tape machine has tracks for, or a tape speed the machine lacks.
    bool prepare(double sampleRate, int maxBlockSize, const StemMixConfig& config);
    void reset();

    // stems[2*i] and stems[2*i+1] are the left and right of stem i; a null pointer is a silent
    // stem. Inputs are not modified. Outputs must not alias any stem.
    void process(const float* const* stems, int numStems, float* outLeft, float* outRight, int numSamples);

    bool isReady() const { return ready; }
    int latencySamples() const { return latency; }
    const char* lastError() const { return error; }

    // Updated once per process() call; read from the audio thread or after it has stopped.
    const std::vector<StemBusState>& stemStates() const { return stemBus.stems(); }
    const IntegratedRealtimeTelemetry& telemetry() const { return meters; }

private:
    struct Delay
    {
        std::vector<float> buffer;
        std::size_t position = 0;
    };

    StemMixConfig config;
    tape::PhysicalTapeMachine machine;
    console::PhysicalConsoleBus bus;
    StemBusManager stemBus;
    IntegratedRealtimeTelemetry meters;

    std::vector<std::vector<float>> scratch;   // one per stem side
    std::vector<float*> tapePointers;          // null for stems that skip the tape
    std::vector<const float*> busPointers;
    std::vector<Delay> bypassDelays;           // one per stem side, used only by stems that skip the tape
    int maxBlock = 512;
    int tapeLatency = 0;
    int latency = 0;
    bool ready = false;
    const char* error = "";
};
