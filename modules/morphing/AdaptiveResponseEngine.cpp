#include "AdaptiveResponseEngine.h"

AdaptiveResponse AdaptiveResponseEngine::evaluate(const SessionStateFingerprint& session) const
{
    AdaptiveResponse response;

    response.suggestedDrive =
        0.12f + session.averageDynamicDensity * 0.3f;

    response.suggestedTransformerWeight =
        0.18f + session.lowFrequencyEnergy * 0.4f;

    response.suggestedTapeMemory =
        0.08f + session.averageRailStress * 0.35f;

    return response;
}