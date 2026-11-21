#pragma once
// Tape machine model driven by a physical MachineSpec.
//
// Signal path per channel (levels normalised so 1.0 = operating level):
//
//   input electronics   high-pass, optional input transformer
//   -- 4x oversampled --
//   record curve        standard flux curve for the speed (NAB / IEC / AES)
//   pre-emphasis        boost that compensates short-wavelength loss
//   linearizer          optional pre-distortion, when the machine has one
//   magnetisation       anhysteretic saturation, calibrated to a third-harmonic figure
//   wavelength loss     exact inverse of the pre-emphasis, so small signals are unchanged
//   tape noise          optional
//   transport           wow, flutter and scrape as a modulated delay, shared by all channels
//   reproduce curve     exact inverse of the record curve
//   high-frequency limit
//   -- back to base rate --
//   reproduce head      head bump at a fixed wavelength, so it moves with tape speed
//   output electronics  optional output transformer, high-pass, line-amp limit
//
// prepare() allocates and runs calibration; call it off the audio thread.
// process() does not allocate, lock or perform I/O.

#include "TapeDspBlocks.h"
#include "TapeMachineSpec.h"
#include <vector>

namespace tape
{
    class PhysicalTapeMachine
    {
    public:
        // Returns false (and leaves the machine bypassed) if the spec has no entry for settings.ips.
        bool prepare(double sampleRate, int maxBlockSize, int numChannels, const MachineSpec& spec, const RunSettings& settings);
        void reset();

        // channels[c] points to numSamples samples, processed in place.
        void process(float* const* channels, int numChannels, int numSamples);

        int latencySamples() const { return latency; }
        bool isReady() const { return ready; }

        // Calibration results, for reporting.
        double saturationDrive() const { return satDrive; }
        double linearizerAmount() const { return linAmount; }
        double recordLossCornerHz() const { return recordLossHz; }
        double headBumpHz() const { return bumpHz; }

    private:
        struct Channel
        {
            dsp::FirstOrder inHp, outHp, inXfmrFlux, outXfmrFlux;
            dsp::Oversampler4x os;
            dsp::FirstOrder recLf, recHf, preEmph, waveLoss, repHf, repLf;
            dsp::Biquad hfLimit, bump, bumpDip, bumpHp;
            dsp::DelayLine delay;
            dsp::Noise noise;
            void reset();
        };

        // Record chain -> saturation -> loss -> reproduce chain at the oversampled rate,
        // without transport or noise. Used both for audio and for calibration.
        struct Core
        {
            dsp::FirstOrder recLf, recHf, preEmph, waveLoss, repHf, repLf;
            double drive = 0.3, lin = 0.0;
            inline double saturate(double u) const
            {
                const double v = std::clamp(drive * u, -1.0, 1.0);
                const double pre = u + lin * v * v * v / (3.0 * drive);
                return std::tanh(drive * pre) / drive;
            }
        };

        void configureCore(Core& core, const SpeedSpec& speed, double fsOs) const;
        double measureHd3(Core core, double levelDb, double fsOs, double* phasor = nullptr) const;
        void calibrateSaturation(const SpeedSpec& speed, double fsOs, bool linearizerOn);
        inline double transformer(dsp::FirstOrder& flux, double x) const;

        MachineSpec machine;
        RunSettings run;
        std::vector<Channel> channels;

        // Transport, shared by all channels.
        struct SineComponent { double phase = 0.0, increment = 0.0, amplitude = 0.0; };
        std::vector<SineComponent> sines;
        dsp::Noise scrapeNoise;
        dsp::FirstOrder scrapeHp, scrapeLp;
        double scrapeGain = 0.0;
        double delayAccumulator = 0.0, delayLeak = 1.0;
        std::vector<double> delayBlock;

        double fs = 48000.0, fsOs = 192000.0;
        double inputGain = 1.0, outputGain = 1.0;
        double satDrive = 0.3, linAmount = 0.0;
        double recordLossHz = 0.0, bumpHz = 0.0;
        double xfmrK = 0.0, outputLimit = 15.8;
        double noiseGain = 0.0;
        int latency = 0;
        bool ready = false;

        static constexpr double kBaseDelayOs = 256.0;  // oversampled samples
        static constexpr double kXfmrFluxHz = 5.0;
    };
}
