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

#include <algorithm>
#include <cmath>
#include <vector>

namespace console
{
    // Filter memory of one stage on one channel.
    struct StageState
    {
        tape::dsp::FirstOrder inFlux, outFlux, inHp, outHp;
        tape::dsp::Biquad hf;
        void reset();
    };

    // One stage's calibrated behaviour at the oversampled rate, in units of nominal level.
    // Shared by PhysicalConsoleStage and PhysicalConsoleBus.
    class StageModel
    {
    public:
        // Solves the curve from the spec. Not for the audio thread. False if the spec is unusable.
        bool calibrate(const StageSpec& spec, double oversampledRate);
        void configure(StageState& state) const;

        inline double run(StageState& s, double x) const
        {
            if (spec.inputTransformer)
            {
                // Magnetising nonlinearity: a bounded cubic of the core flux, largest at low frequencies.
                const double phi = s.inFlux.process(x);
                x = s.inHp.process(x - phi * std::min(k * phi * phi, 0.5));
            }

            // Amplifier: nearly straight until close to the ceiling, then a firm limit, plus
            // small even- and odd-order terms that set the distortion well below the knee.
            x = std::clamp(x, -1.0e3 * ceiling, 1.0e3 * ceiling);
            const double r = std::abs(x) / ceiling;
            const double soft = (r * r) / (1.0 + r * r);
            double y = x - a3 * x * soft;
            if (r > 1.0e-3)
                y /= std::pow(1.0 + std::pow(r, p), 1.0 / p);
            y += a2 * ceiling * soft;

            if (spec.outputTransformer)
            {
                const double phi = s.outFlux.process(y);
                y -= phi * std::min(k * phi * phi, 0.5);
            }
            y = s.outHp.process(y);   // output coupling; also removes the offset of the even term
            return s.hf.process(y);
        }

        const StageSpec& stageSpec() const { return spec; }
        double level(double dbu) const { return tape::dsp::dbToGain(dbu - spec.nominalLevelDbu); }

        double ceiling = 16.0;  // output limit
        double p = 12.0;        // knee sharpness
        double a2 = 0.0;        // even-order amount
        double a3 = 0.0;        // odd-order amount below the knee
        double k = 0.0;         // transformer magnetising nonlinearity

    private:
        // Harmonics 2..5 of a sine through one stage, as a fraction of the fundamental.
        // `only` picks one harmonic (0 = all).
        double measure(double hz, double amplitude, int only) const;

        StageSpec spec;
        double fsOs = 192000.0;
    };

    class PhysicalConsoleStage
    {
    public:
        bool prepare(double sampleRate, int numChannels, const StageSpec& spec, const StageRunSettings& settings);
        void reset();
        void process(float* const* channels, int numChannels, int numSamples);

        bool isReady() const { return ready; }
        // Whole samples of delay, valid after prepare(): the oversampling filters plus each
        // stage's bandwidth filter. 34 for one stage at 48 kHz.
        int latencySamples() const { return latency; }

        // Calibration results, for reports and tests.
        double kneeExponent() const { return model.p; }
        double evenAmount() const { return model.a2; }
        double oddAmount() const { return model.a3; }
        double transformerConstant() const { return model.k; }

    private:
        struct Channel
        {
            tape::dsp::Oversampler4x os;
            std::vector<StageState> stages;
            tape::dsp::Noise noise;
        };

        StageModel model;
        StageRunSettings settings;
        std::vector<Channel> channels;
        double fs = 48000.0;
        double inputGain = 1.0, outputGain = 1.0, noiseGain = 0.0;
        int latency = 34;
        bool ready = false;
    };

    // Delay in whole samples of `stages` stages in series inside one oversampled domain:
    // 34.2 samples for the resampling filters plus the low-frequency group delay of each
    // stage's second-order bandwidth filter.
    int stagePathLatency(double sampleRate, double inverseCornerSum);  // sum of 1/hfCornerHz over the stages

    // White-noise gain at the base rate whose 22 Hz to 22 kHz part equals the spec's figure.
    double stageNoiseGain(const StageSpec& spec, double sampleRate);
}
