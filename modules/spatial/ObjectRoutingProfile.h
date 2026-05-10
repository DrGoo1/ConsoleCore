#pragma once
#include <string>
#include <vector>

struct SpatialObject
{
    std::string name;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct ObjectRoutingProfile
{
    std::string profileName;
    std::vector<SpatialObject> objects;
};