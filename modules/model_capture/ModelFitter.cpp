#include "ModelFitter.h"
#include <algorithm>
#include <cmath>

AnalogPhysicsParameters ModelFitter::fitBasicProfile(const ExtractedFeatures& f)
{
    AnalogPhysicsParameters p;

    p.drive = std::clamp(f.estimatedThd * 8.0f, 0.05f, 0.70f);
    p.railStressAmount = std::clamp((f.crestFactor - 1.5f) * 0.15f, 0.10f, 0.85f);
    p.railRecoveryMs = std::clamp(f.recoveryMs, 80.0f, 1200.0f);
    p.transformerWeight = std::clamp(f.lowBandWeight * 0.75f, 0.05f, 0.85f);
    p.tapeMemoryAmount = std::clamp(f.recoveryMs / 1200.0f, 0.02f, 0.85f);
    p.crosstalkAmount = std::clamp(std::pow(10.0f, f.estimatedCrosstalkDb / 20.0f) * 10.0f, 0.0f, 0.20f);
    p.impedanceInteraction = std::clamp(0.15f + p.railStressAmount * 0.35f, 0.05f, 0.75f);
    p.lowFrequencyLoadBias = std::clamp(f.lowBandWeight, 0.05f, 0.75f);
    p.highFrequencySlewSoftening = std::clamp(f.highBandSoftening, 0.02f, 0.60f);
    p.outputTrimDb = -0.25f;

    return p;
}