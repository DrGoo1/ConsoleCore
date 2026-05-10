#include "../../modules/control/VirtualConsoleStrip.h"
#include "../../modules/control/MeterBridgeState.h"
#include "../../modules/control/RemoteConsoleSession.h"
#include <iostream>

int main()
{
    VirtualConsoleStrip strip;
    strip.stemId = 1;
    strip.name = "Drums";
    strip.trimDb = -1.5f;
    strip.meterPeak = 0.82f;

    MeterBridgeState bridge;
    bridge.channels.push_back({0.82f, 0.24f});
    bridge.channels.push_back({0.61f, 0.18f});

    RemoteConsoleSession session;
    session.connectedClients.push_back({"Mix Engineer", "192.168.1.50"});

    std::cout << "ConsoleCore Control Surface CLI\n";
    std::cout << "Strip: " << strip.name << "\n";
    std::cout << "Meter Channels: " << bridge.channels.size() << "\n";
    std::cout << "Remote Clients: " << session.connectedClients.size() << "\n";

    return 0;
}