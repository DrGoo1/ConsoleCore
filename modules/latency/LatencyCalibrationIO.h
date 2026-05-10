#pragma once
#include "LatencyCalibration.h"
#include <string>

class LatencyCalibrationIO
{
public:
    static bool save(const LatencyCalibration& calibration, const std::string& path);
    static bool load(LatencyCalibration& calibration, const std::string& path);
};