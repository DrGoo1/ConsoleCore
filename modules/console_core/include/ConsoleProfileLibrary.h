#pragma once
#include "ConsoleModelProfile.h"
#include "TopologyTypes.h"
#include <vector>

namespace consolecore {

struct ConsoleProfileLibrary {
    static ConsoleModelProfile makeProfile(ConsoleFamily family);
    static std::vector<ConsoleModelProfile> defaultProfiles();
};

} // namespace consolecore
