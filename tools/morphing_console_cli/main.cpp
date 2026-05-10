#include "../../modules/morphing/ConsoleMorphTarget.h"
#include "../../modules/morphing/ParameterInterpolator.h"
#include "../../modules/morphing/AdaptiveResponseEngine.h"
#include "../../modules/morphing/TemporalSceneEvolution.h"
#include <iostream>

int main()
{
    ConsoleMorphTarget morph;
    morph.sourceProfile = "British Class-A Discrete";
    morph.targetProfile = "Modern Ultra-Clean";
    morph.interpolationAmount = 0.35f;

    const float interpolated =
        ParameterInterpolator::lerp(0.2f, 0.7f, morph.interpolationAmount);

    SessionStateFingerprint session;
    session.averageRailStress = 0.62f;
    session.averageDynamicDensity = 0.48f;
    session.lowFrequencyEnergy = 0.55f;

    AdaptiveResponseEngine engine;
    auto response = engine.evaluate(session);

    TemporalSceneEvolution evolution;
    evolution.addPoint({0.0f, 0.0f});
    evolution.addPoint({45.0f, 0.5f});
    evolution.addPoint({90.0f, 1.0f});

    std::cout << "ConsoleCore Morphing CLI\n";
    std::cout << "Interpolated Value: " << interpolated << "\n";
    std::cout << "Suggested Drive: " << response.suggestedDrive << "\n";
    std::cout << "Scene Points: " << evolution.points().size() << "\n";

    return 0;
}