#include "LoopbackRoutingProfileIO.h"
#include <fstream>

bool LoopbackRoutingProfileIO::save(const LoopbackRoutingProfile& p, const std::string& path)
{
    std::ofstream out(path);
    if (!out) return false;

    out << "{\n";
    out << "  \"profileName\": \"" << p.profileName << "\",\n";
    out << "  \"inputDeviceName\": \"" << p.inputDeviceName << "\",\n";
    out << "  \"outputDeviceName\": \"" << p.outputDeviceName << "\",\n";
    out << "  \"sampleRate\": " << p.sampleRate << ",\n";
    out << "  \"bufferSize\": " << p.bufferSize << ",\n";
    out << "  \"reportedRoundtripLatencySamples\": " << p.reportedRoundtripLatencySamples << "\n";
    out << "}\n";
    return true;
}

bool LoopbackRoutingProfileIO::load(LoopbackRoutingProfile& p, const std::string& path)
{
    std::ifstream in(path);
    if (!in) return false;

    // Phase scaffold: full JSON parsing should be implemented with JUCE JSON or nlohmann/json.
    p.profileName = path;
    return true;
}