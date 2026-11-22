#pragma once
// Physical description of a tape machine, in engineering units.
//
// This replaces 0..1 "amount" knobs with quantities that can be read off a
// maintenance manual, derived from geometry, or stated as an assumption.
// Every numeric field that shapes the sound must have an entry in `evidence`
// saying which of those it is. See docs/TAPE_MACHINE_MODELS.md.

#include <map>
#include <string>
#include <vector>

namespace tape
{
    // A published tolerance for overall record/reproduce response.
    struct ResponseBand
    {
        double loHz = 0.0;
        double hiHz = 0.0;
        double plusDb = 0.0;   // allowed above the reference frequency level
        double minusDb = 0.0;  // allowed below (positive number)
    };

    struct SpeedSpec
    {
        double ips = 15.0;
        std::string equalization = "NAB";
        double hfTimeConstantUs = 50.0;    // standard flux curve, high frequency
        double lfTimeConstantUs = 3180.0;  // 0 = no low-frequency boost in the standard
        double playbackHfCornerHz = 24000.0;
        double flutterLowBandRmsPercent = 0.0;  // 0.5 Hz - 300 Hz
        double scrapeFlutterRmsPercent = 0.0;   // 300 Hz - 5 kHz
        double snrDb = 60.0;                    // unweighted, re reference level
        double referenceHz = 1000.0;            // frequency the tolerance bands refer to
        std::vector<ResponseBand> publishedResponse;
        double publishedFlutterMaxPercent = 0.0;  // 0 = none published
    };

    struct FlutterComponent
    {
        double hzAt15Ips = 1.0;  // rotation-locked: scales with tape speed
        double weight = 1.0;
    };

    struct MachineSpec
    {
        std::string id;
        std::string displayName;
        std::string referenceMachine;
        std::string status;

        int tracks = 24;
        double tapeWidthInches = 2.0;
        double trackWidthInches = 0.043;

        // Level calibration: digital level that equals the machine's operating level (0 VU).
        double referenceDbfs = -18.0;

        // Tape and record process.
        double coatingThicknessMicrons = 12.5;
        double hd3PercentAtCal = 3.0;   // third harmonic at 1 kHz ...
        double hd3CalLevelDb = 6.0;     // ... this far above operating level
        bool hasLinearizer = false;
        double linearizerHd3Percent = 0.8;
        double biasFrequencyHz = 0.0;   // informational: bias is not simulated as a signal
        // Bias alignment point: from the bias giving peak output at overbiasReferenceHz
        // (at 15 in/s), bias is raised until that output falls by overbiasDb. Zero means
        // unknown, in which case the bias control changes distortion only.
        double overbiasDb = 0.0;
        double overbiasReferenceHz = 10000.0;
        double biasRangeToNominalDb = 3.0;  // bias increase from peak to nominal: an assumption

        // Set when a tape formula has replaced the machine's own tape values.
        std::string formulaId;
        double publishedHd3PercentAtOperatingLevel = 0.0;  // 0 = none; a check, not an input

        // Reproduce head.
        double headBumpWavelengthInches = 0.30;
        double headBumpGainDb = 1.0;

        // Electronics.
        bool inputTransformer = false;
        bool outputTransformer = false;
        double transformerHd3Percent = 0.8;  // at transformerCalHz and maxOutputDb
        double transformerCalHz = 30.0;
        double maxOutputDb = 24.0;           // above operating level
        double electronicsHighpassHz = 5.0;

        std::vector<FlutterComponent> flutterComponents;
        std::vector<SpeedSpec> speeds;

        // "field.path" -> "published: ..." | "standard: ..." | "derived: ..." | "assumed: ..."
        std::map<std::string, std::string> evidence;

        const SpeedSpec* findSpeed(double ips) const
        {
            for (const auto& s : speeds)
                if (s.ips > ips - 0.01 && s.ips < ips + 0.01)
                    return &s;
            return nullptr;
        }
    };

    struct RunSettings
    {
        double ips = 15.0;
        double inputDriveDb = 0.0;   // extra level into the machine; output is compensated
        bool flutterEnabled = true;
        bool noiseEnabled = false;   // off by default: every profile's noise level is an assumption so far
        bool linearizerEnabled = true;  // only meaningful when the machine has one
        // Bias relative to the aligned setting, -3..+3 dB. The machine's equalization stays
        // as aligned, so under-bias brightens and distorts and over-bias dulls. The shape of
        // both effects is an assumption; only the alignment point comes from documents.
        double biasOffsetDb = 0.0;
    };

    // A tape stock. Levels are relative to an operating level of 185 nWb/m.
    struct TapeFormula
    {
        std::string id;
        std::string displayName;
        std::string referenceTape;
        std::string status;
        double coatingThicknessMicrons = 12.7;
        double molDb = 12.0;                     // level of 3 % third harmonic at 1 kHz
        double hd3PercentAtOperatingLevel = 0.0; // published check value, 0 = none
        double overbiasDb = 3.0;
        double overbiasReferenceHz = 10000.0;
        std::map<std::string, std::string> evidence;  // coatingThicknessMicrons, molDb, overbias
    };

    // The machine with this tape threaded: tape values and their evidence come from the
    // formula; heads, electronics and transport stay the machine's.
    inline MachineSpec withFormula(MachineSpec machine, const TapeFormula& formula)
    {
        machine.formulaId = formula.id;
        machine.coatingThicknessMicrons = formula.coatingThicknessMicrons;
        machine.hd3PercentAtCal = 3.0;
        machine.hd3CalLevelDb = formula.molDb;
        machine.overbiasDb = formula.overbiasDb;
        machine.overbiasReferenceHz = formula.overbiasReferenceHz;
        machine.publishedHd3PercentAtOperatingLevel = formula.hd3PercentAtOperatingLevel;
        auto note = [&](const char* key) { const auto it = formula.evidence.find(key); return it != formula.evidence.end() ? it->second : std::string("assumed: no note in the formula"); };
        machine.evidence["tape.coatingThicknessMicrons"] = note("coatingThicknessMicrons");
        machine.evidence["tape.hd3PercentAtCal"] = note("molDb");
        machine.evidence["tape.hd3CalLevelDb"] = note("molDb");
        machine.evidence["tape.overbias"] = note("overbias");
        return machine;
    }
}
