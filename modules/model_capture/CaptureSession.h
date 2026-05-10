#pragma once
#include <string>
#include <vector>

struct CaptureFilePair
{
    std::string testName;
    std::string dryPath;
    std::string wetPath;
};

struct CaptureSession
{
    std::string sessionName;
    std::string sampleRate;
    std::string bitDepth;
    std::string hardwareNotes;
    std::string converterNotes;
    std::string calibrationNotes;
    std::vector<CaptureFilePair> files;
};