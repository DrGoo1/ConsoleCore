#pragma once
// A console mix bus: each stereo stem passes through its own channel stage, the stems are
// summed, and the sum passes through a shared bus stage. Because the bus stage works on the
// sum, its distortion depends on everything playing at once: stems intermodulate, and a
// loud stem changes how a quiet one is treated. Processing stems separately cannot do that.
//
// Everything runs inside one oversampled domain, so latency does not grow with the number
// of stems: it is one resampling delay plus the two stages' small filter delays.
//
// Documentation-derived, like the stages it is made of. Not validated against hardware.
//
// Real-time contract: prepare() allocates and calibrates; process() does not allocate.

#include "PhysicalConsoleStage.h"

namespace console
{
    struct BusRunSettings
    {
        double channelDriveDb = 0.0;  // level into every channel stage; compensated after it
        double busDriveDb = 0.0;      // level into the bus stage; compensated after it
        bool noiseEnabled = false;
    };

    class PhysicalConsoleBus
    {
    public:
        static constexpr int kMaxStems = 64;

        // Stems are stereo. The channel and bus specs must share referenceDbfs and nominal level.
        bool prepare(double sampleRate, int numStems, const StageSpec& channelSpec, const StageSpec& busSpec, const BusRunSettings& settings);
        void reset();

        // stems[2*i] and stems[2*i+1] are the left and right of stem i (a null pointer is a
        // silent stem). Writes the mix to outLeft/outRight, which must not alias any stem.
        void process(const float* const* stems, int numStems, float* outLeft, float* outRight, int numSamples);

        bool isReady() const { return ready; }
        int stemCount() const { return static_cast<int>(stems.size()); }
        int latencySamples() const { return latency; }   // valid after prepare(); 35 at 48 kHz for the class-A profile

    private:
        struct Side
        {
            tape::dsp::Oversampler4x up;   // only its upsampling half is used
            StageState state;
        };
        struct Stem
        {
            Side side[2];
        };
        struct Output
        {
            tape::dsp::Oversampler4x down;  // only its downsampling half is used
            StageState state;
            tape::dsp::Noise noise;
        };

        StageModel channelModel, busModel;
        std::vector<Stem> stems;
        Output out[2];
        double inputGain = 1.0, afterChannel = 1.0, outputGain = 1.0, noiseGain = 0.0;
        int latency = 35;
        bool ready = false;
    };
}
