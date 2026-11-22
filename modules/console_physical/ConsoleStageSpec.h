#pragma once
// Description of one console amplifier stage (a line amplifier with its transformers),
// with a note for every sound-shaping value saying where it came from.
// Levels are in dBu. Standard library only.

#include <map>
#include <string>
#include <vector>

namespace console
{
    struct ResponseBand
    {
        double loHz = 20.0, hiHz = 20000.0, plusDb = 0.5, minusDb = 0.5;
    };

    struct StageSpec
    {
        std::string id;
        std::string displayName;
        std::string referenceModule;
        std::string status;

        // Level calibration: this digital level equals nominalLevelDbu.
        double referenceDbfs = -18.0;
        double nominalLevelDbu = 4.0;

        // Amplifier.
        double thdPercentAtCal = 0.05;       // total harmonic distortion at 1 kHz ...
        double thdCalLevelDbu = 20.0;        // ... at this output level
        double maxOutputDbu = 26.0;          // level the module is rated to deliver
        double thdPercentAtMaxOutput = 1.0;  // distortion taken to define that rating
        double clipCeilingDbu = 28.0;        // level the output can never exceed
        // Share of the calibration-level distortion that is even-order (second harmonic).
        // 1 suits a single-ended stage, a small value a push-pull one.
        double evenHarmonicShare = 1.0;

        // Transformers.
        bool inputTransformer = true;
        bool outputTransformer = true;
        double transformerHd3PercentAtCal = 0.04;  // third harmonic from the iron ...
        double transformerCalHz = 50.0;            // ... at this frequency and thdCalLevelDbu
        double lfCornerHz = 4.5;                   // low-frequency corner of each coupling

        // Bandwidth and noise.
        double hfCornerHz = 40000.0;   // -3 dB point
        double noiseDbu = -83.0;       // 22 Hz to 22 kHz

        // Published limits, used only by the bench.
        std::vector<ResponseBand> publishedResponse;
        double publishedThdMaxPercent = 0.0;   // 0 = none
        double publishedThdLoHz = 50.0, publishedThdHiHz = 10000.0;
        double publishedThdLevelDbu = 0.0;     // 0 = at thdCalLevelDbu

        // "field" -> "published: ..." | "standard: ..." | "derived: ..." | "assumed: ..."
        std::map<std::string, std::string> evidence;
    };

    struct StageRunSettings
    {
        double driveDb = 0.0;      // extra level into the stage; output is compensated
        int stages = 1;            // identical stages in series, 1..4 (channel amp, bus amp, ...)
        bool noiseEnabled = false;
    };
}
