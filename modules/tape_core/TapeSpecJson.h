#pragma once
// Loads a MachineSpec from a "tapecore.machine.v2" JSON profile.
// Standard library only. Not for use on the audio thread.

#include "TapeMachineSpec.h"
#include <string>

namespace tape
{
    // On failure returns false and sets `error` to a message naming the problem.
    bool parseMachineSpec(const std::string& jsonText, MachineSpec& out, std::string& error);
    bool loadMachineSpecFile(const std::string& path, MachineSpec& out, std::string& error);

    // Checks a loaded spec: plausible ranges, at least one speed, and an evidence
    // entry for every field that shapes the sound. Returns problems found.
    std::vector<std::string> validateMachineSpec(const MachineSpec& spec);

    // Tape formulas: "tapecore.formula.v1" JSON.
    bool parseTapeFormula(const std::string& jsonText, TapeFormula& out, std::string& error);
    bool loadTapeFormulaFile(const std::string& path, TapeFormula& out, std::string& error);
    std::vector<std::string> validateTapeFormula(const TapeFormula& formula);
}
