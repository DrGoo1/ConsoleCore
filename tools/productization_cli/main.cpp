#include "../../modules/product/UserProfile.h"
#include "../../modules/product/LicenseState.h"
#include "../../modules/product/DiagnosticsBundle.h"
#include "../../modules/product/SessionPortability.h"
#include <iostream>

int main()
{
    UserProfile user;
    user.displayName = "Console Engineer";
    user.preferredConsole = "British Class-A Discrete";

    LicenseState license;
    license.activated = true;
    license.licenseId = "CC-TEST-001";

    DiagnosticsBundle diagnostics;
    diagnostics.appVersion = "0.9.0-beta";
    diagnostics.averageCpuLoad = 14.2f;
    diagnostics.activeModules = {
        "AnalogPhysicsCore",
        "SpatialEngine",
        "RuntimeController"
    };

    PortableSession session;
    session.sessionName = "Demo Mix";
    session.consoleProfile = "British Class-A Discrete";

    std::cout << "ConsoleCore Productization CLI\n";
    std::cout << "User: " << user.displayName << "\n";
    std::cout << "License Active: " << (license.activated ? "yes" : "no") << "\n";
    std::cout << "CPU Load: " << diagnostics.averageCpuLoad << "%\n";
    std::cout << "Portable Session: " << session.sessionName << "\n";

    return 0;
}