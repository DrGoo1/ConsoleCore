#include "CaptureSessionIO.h"
#include <fstream>
#include <sstream>

// Minimal JSON-ish writer/reader scaffold.
// Copilot should replace with nlohmann/json or JUCE var when integrated.

bool CaptureSessionIO::saveJson(const CaptureSession& session, const std::string& path)
{
    std::ofstream out(path);
    if (!out) return false;

    out << "{\n";
    out << "  \"sessionName\": \"" << session.sessionName << "\",\n";
    out << "  \"sampleRate\": \"" << session.sampleRate << "\",\n";
    out << "  \"bitDepth\": \"" << session.bitDepth << "\",\n";
    out << "  \"hardwareNotes\": \"" << session.hardwareNotes << "\",\n";
    out << "  \"converterNotes\": \"" << session.converterNotes << "\",\n";
    out << "  \"calibrationNotes\": \"" << session.calibrationNotes << "\",\n";
    out << "  \"files\": [\n";
    for (size_t i = 0; i < session.files.size(); ++i)
    {
        const auto& f = session.files[i];
        out << "    { \"testName\": \"" << f.testName << "\", \"dryPath\": \"" << f.dryPath << "\", \"wetPath\": \"" << f.wetPath << "\" }";
        if (i + 1 < session.files.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";
    return true;
}

bool CaptureSessionIO::loadJson(CaptureSession& session, const std::string& path)
{
    std::ifstream in(path);
    if (!in) return false;

    // Phase scaffold: validate file exists and store name.
    session.sessionName = path;
    return true;
}