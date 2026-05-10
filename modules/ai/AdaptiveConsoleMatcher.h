#pragma once
#include "SessionFingerprint.h"
#include <string>

struct MatchResult
{
    std::string profileName;
    float similarity = 0.0f;
};

class AdaptiveConsoleMatcher
{
public:
    MatchResult suggestProfile(const SessionFingerprint& session) const;
};