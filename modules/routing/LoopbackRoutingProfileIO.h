#pragma once
#include "LoopbackRoutingProfile.h"
#include <string>

class LoopbackRoutingProfileIO
{
public:
    static bool save(const LoopbackRoutingProfile& profile, const std::string& path);
    static bool load(LoopbackRoutingProfile& profile, const std::string& path);
};