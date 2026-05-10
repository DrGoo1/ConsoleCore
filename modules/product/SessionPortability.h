#pragma once
#include <string>
#include <vector>

struct PortableSession
{
    std::string sessionName;
    std::vector<std::string> routingProfiles;
    std::vector<std::string> snapshots;
    std::string consoleProfile;
};