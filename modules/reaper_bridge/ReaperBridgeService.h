#pragma once
#include <string>

class ReaperBridgeService
{
public:
    bool launchHiddenReaper();
    bool connectOSC(const std::string& host, int port);
    void synchronizeTransport();

private:
    bool connected = false;
};