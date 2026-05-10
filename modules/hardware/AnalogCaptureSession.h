#pragma once
#include "HardwareDevice.h"
#include "PatchChainDefinition.h"
#include "ConverterCalibrationProfile.h"
#include <vector>
#include <string>

struct AnalogCaptureSession
{
    std::string sessionName;
    std::vector<HardwareDevice> devices;
    PatchChainDefinition patchChain;
    ConverterCalibrationProfile converterProfile;
};