#pragma once
#include <string>

enum class LicenseTier
{
    Free,
    Standard,
    Professional,
    Enterprise
};

struct LicenseState
{
    LicenseTier tier = LicenseTier::Free;
    bool activated = false;
    std::string licenseId;
    std::string registeredEmail;
};