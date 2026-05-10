#pragma once
#include "../tape_core/TapeMachineProfile.h"

struct AnalogPathSettings
{
    bool tapeTrackEnabled = false;
    bool consoleEnabled = true;
    bool printTapeEnabled = false;

    int activeStemCount = 2;

    float inputTrimDb = 0.0f;
    float outputTrimDb = 0.0f;

    TapeMachineProfile trackTapeProfile;
    TapeMachineProfile printTapeProfile;
};
