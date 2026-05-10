#pragma once
#include <string>
#include <vector>

struct RemoteClient
{
    std::string name;
    std::string address;
};

struct RemoteConsoleSession
{
    std::vector<RemoteClient> connectedClients;
};