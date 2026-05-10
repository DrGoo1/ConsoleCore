#pragma once
#include "SessionStateFingerprint.h"

struct AdaptiveResponse
{
    float suggestedDrive = 0.0f;
    float suggestedTransformerWeight = 0.0f;
    float suggestedTapeMemory = 0.0f;
};

class AdaptiveResponseEngine
{
public:
    AdaptiveResponse evaluate(const SessionStateFingerprint& session) const;
};