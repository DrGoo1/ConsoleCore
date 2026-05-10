#include "AdaptiveConsoleMatcher.h"

MatchResult AdaptiveConsoleMatcher::suggestProfile(const SessionFingerprint& session) const
{
    MatchResult result;

    if (session.averageRailStress > 0.5f)
    {
        result.profileName = "British Class-A Discrete";
        result.similarity = 0.84f;
    }
    else
    {
        result.profileName = "Modern Ultra-Clean";
        result.similarity = 0.76f;
    }

    return result;
}