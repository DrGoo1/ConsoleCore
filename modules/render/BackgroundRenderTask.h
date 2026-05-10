#pragma once
#include <string>

struct BackgroundRenderTask
{
    std::string taskName;
    std::string sourcePath;
    std::string destinationPath;
    bool completed = false;
};