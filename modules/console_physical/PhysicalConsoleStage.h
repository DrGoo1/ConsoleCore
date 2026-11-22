#pragma once
// Console amplifier stage built from a StageSpec: transformer-coupled input, an amplifier
// with a calibrated transfer curve, transformer-coupled output and a bandwidth limit, run
// at four times the sample rate.
//
// Documentation-derived: the curve is solved so the stage shows the profile's distortion at
// the profile's levels. It is not a circuit simulation and was not measured from hardware.
//
// Real-time contract: prepare() allocates and calibrates and must not be called on the
// audio thread. process() does not allocate, lock or throw.

#include "ConsoleStageSpec.h"
#include "../tape_core/TapeDspBlocks.h"

#include <vector>

namespace console
{
    class PhysicalConsoleStage
    {
    public:
        bool prepare(double sampleRate, int numChannels, const StageSpec& spec, const StageRunSettings& settings);
        void reset();
        void process(float* const* channels, int numChannels, int numSamples);

        bool isReady() const { return ready; }
        // The oversampling filters delay the signal by 34.3 samples; hosts take whole samples.
        static int latencySamples() { return 34; }

        // Calibration results, for reports and tests.
        double kneeExponent() const { return curve.p; }
        double evenAmount() const { return curve.a2; }
        double transformerConstant() const { return curve.k; }

    private:
        struct Curve
        {
            double ceiling = 16.0;  // in units of nominal level
            double p = 12.0;        // knee sharpness of the odd-symmetric limit
            double a2 = 0.0;        // even-order amount (single-ended class A asymmetry)
            double k = 0.0;         // transformer magnetising nonlinearity
        };
        struct Stage
        {
            tape::dsp::FirstOrder inFlux, outFlux, inHp, outHp;
            tape::dsp::Biquad hf;
            void reset();
        };
        struct Channel
        {
            tape::dsp::Oversampler4x os;
            std::vector<Stage> stages;
            tape::dsp::Noise noise;
        };

        void configure(Stage& stage) const;
        inline double run(Stage& stage, double x, const Curve& c) const;
        // Harmonics 2..5 of a sine through one stage, as a fraction of the fundamental.
        double measure(const Curve& c, double hz, double level, bool thirdOnly) const;

        StageSpec spec;
        StageRunSettings settings;
        Curve curve;
        std::vector<Channel> channels;
        double fs = 48000.0, fsOs = 192000.0;
        double inputGain = 1.0, outputGain = 1.0, noiseGain = 0.0;
        bool ready = false;
    };
}
