#pragma once
#include <string>

struct CrashReport
{
    std::string timestamp;
    std::string appVersion;
    std::string operatingSystem;
    std::string lastLoadedProfile;
    std::string notes;
};