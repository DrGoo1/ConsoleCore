#include "ConsoleProfileLibrary.h"

namespace consolecore {

ConsoleModelProfile ConsoleProfileLibrary::makeProfile(ConsoleFamily family) {
    ConsoleModelProfile p;
    switch (family) {
        case ConsoleFamily::BritishClassADiscrete:
            p.safeName = "British Class-A Discrete"; p.railRecoveryMs = 420.0f; p.transformerWeight = 0.36f; p.slewSoftening = 0.22f; p.crosstalkDb = -72.0f; p.nominalHeadroomDbu = 24.0f; p.busSagDepth = 0.18f; break;
        case ConsoleFamily::BritishVcaLargeFormat:
            p.safeName = "British VCA Large Format"; p.railRecoveryMs = 260.0f; p.transformerWeight = 0.12f; p.slewSoftening = 0.12f; p.crosstalkDb = -82.0f; p.nominalHeadroomDbu = 26.0f; p.busSagDepth = 0.10f; break;
        case ConsoleFamily::AmericanDiscreteOpAmp:
            p.safeName = "American Discrete Op-Amp"; p.railRecoveryMs = 310.0f; p.transformerWeight = 0.24f; p.slewSoftening = 0.08f; p.crosstalkDb = -76.0f; p.nominalHeadroomDbu = 25.0f; p.busSagDepth = 0.14f; break;
        case ConsoleFamily::GermanTubeBroadcast:
            p.safeName = "German Tube Broadcast"; p.railRecoveryMs = 610.0f; p.transformerWeight = 0.42f; p.slewSoftening = 0.30f; p.crosstalkDb = -68.0f; p.nominalHeadroomDbu = 22.0f; p.busSagDepth = 0.22f; break;
        case ConsoleFamily::GermanSolidStateBroadcast:
            p.safeName = "German Solid-State Broadcast"; p.railRecoveryMs = 330.0f; p.transformerWeight = 0.18f; p.slewSoftening = 0.07f; p.crosstalkDb = -86.0f; p.nominalHeadroomDbu = 27.0f; p.busSagDepth = 0.08f; break;
        case ConsoleFamily::EmiInspiredTubeDesk:
            p.safeName = "EMI-Inspired Tube Desk"; p.railRecoveryMs = 720.0f; p.transformerWeight = 0.46f; p.slewSoftening = 0.34f; p.crosstalkDb = -64.0f; p.nominalHeadroomDbu = 21.0f; p.busSagDepth = 0.25f; break;
        case ConsoleFamily::FlickingerInspiredDiscrete:
            p.safeName = "Flickinger-Inspired Discrete"; p.railRecoveryMs = 390.0f; p.transformerWeight = 0.30f; p.slewSoftening = 0.16f; p.crosstalkDb = -70.0f; p.nominalHeadroomDbu = 24.0f; p.busSagDepth = 0.20f; break;
        case ConsoleFamily::CustomHybrid:
            p.safeName = "Custom Hybrid"; p.railRecoveryMs = 450.0f; p.transformerWeight = 0.26f; p.slewSoftening = 0.15f; p.crosstalkDb = -78.0f; p.nominalHeadroomDbu = 25.0f; p.busSagDepth = 0.15f; break;
        case ConsoleFamily::ModernClean:
        default:
            p.safeName = "Modern Ultra-Clean"; p.railRecoveryMs = 180.0f; p.transformerWeight = 0.04f; p.slewSoftening = 0.02f; p.crosstalkDb = -96.0f; p.nominalHeadroomDbu = 30.0f; p.busSagDepth = 0.04f; break;
    }
    return p;
}

std::vector<ConsoleModelProfile> ConsoleProfileLibrary::defaultProfiles() {
    return { makeProfile(ConsoleFamily::ModernClean), makeProfile(ConsoleFamily::BritishClassADiscrete), makeProfile(ConsoleFamily::BritishVcaLargeFormat), makeProfile(ConsoleFamily::AmericanDiscreteOpAmp), makeProfile(ConsoleFamily::GermanTubeBroadcast), makeProfile(ConsoleFamily::GermanSolidStateBroadcast), makeProfile(ConsoleFamily::EmiInspiredTubeDesk), makeProfile(ConsoleFamily::FlickingerInspiredDiscrete), makeProfile(ConsoleFamily::CustomHybrid) };
}

} // namespace consolecore
