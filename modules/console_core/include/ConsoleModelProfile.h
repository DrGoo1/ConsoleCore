#pragma once

#include <string>

namespace consolecore
{
struct ConsoleModelProfile
{
    std::string safeName { "Modern Clean Reference" };
    double railRecoveryMs { 250.0 };
    double busSagDepth { 0.0 };
    double transformerWeight { 0.0 };
    double slewSoftening { 0.0 };
    double crosstalkDb { -96.0 };
    double nominalHeadroomDbu { 24.0 };
};
}
