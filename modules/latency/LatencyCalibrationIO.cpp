#include "LatencyCalibrationIO.h"
#include <fstream>

bool LatencyCalibrationIO::save(const LatencyCalibration& c, const std::string& path)
{
    std::ofstream out(path);
    if (!out) return false;

    out << "{\n";
    out << "  \"profileName\": \"" << c.profileName << "\",\n";
    out << "  \"dawName\": \"" << c.dawName << "\",\n";
    out << "  \"sampleRate\": " << c.sampleRate << ",\n";
    out << "  \"bufferSize\": " << c.bufferSize << ",\n";
    out << "  \"measuredRoundtripSamples\": " << c.measuredRoundtripSamples << ",\n";
    out << "  \"manualOffsetSamples\": " << c.manualOffsetSamples << "\n";
    out << "}\n";
    return true;
}

bool LatencyCalibrationIO::load(LatencyCalibration& c, const std::string& path)
{
    std::ifstream in(path);
    if (!in) return false;
    c.profileName = path;
    return true;
}