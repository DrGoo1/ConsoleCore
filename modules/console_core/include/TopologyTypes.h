#pragma once
#include <string>
#include <vector>

namespace consolecore {

enum class ConsoleFamily {
    ModernClean,
    BritishClassADiscrete,
    BritishVcaLargeFormat,
    AmericanDiscreteOpAmp,
    GermanTubeBroadcast,
    GermanSolidStateBroadcast,
    EmiInspiredTubeDesk,
    FlickingerInspiredDiscrete,
    CustomHybrid
};

struct StemConfig {
    int stemId = 0;
    std::string name = "Stem";
    int inputLeft = 0;
    int inputRight = 1;
    float trimDb = 0.0f;
    float pan = 0.0f;
    bool muted = false;
    bool soloed = false;
};

struct ConsoleTopology {
    int inputChannelCount = 16;
    int outputChannelCount = 2;
    std::vector<StemConfig> stems;
};

} // namespace consolecore
