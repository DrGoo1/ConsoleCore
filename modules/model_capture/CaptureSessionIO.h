#pragma once
#include "CaptureSession.h"
#include <string>

class CaptureSessionIO
{
public:
    static bool saveJson(const CaptureSession& session, const std::string& path);
    static bool loadJson(CaptureSession& session, const std::string& path);
};