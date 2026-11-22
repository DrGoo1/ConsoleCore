#pragma once
// Loads a StageSpec from a "consolecore.stage.v1" JSON profile. Not for the audio thread.

#include "ConsoleStageSpec.h"
#include <string>
#include <vector>

namespace console
{
    bool parseStageSpec(const std::string& jsonText, StageSpec& out, std::string& error);
    bool loadStageSpecFile(const std::string& path, StageSpec& out, std::string& error);

    // Plausible ranges and an evidence note for every field that shapes the sound.
    std::vector<std::string> validateStageSpec(const StageSpec& spec);
}
