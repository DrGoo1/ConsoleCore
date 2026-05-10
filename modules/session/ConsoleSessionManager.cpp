#include "ConsoleSessionManager.h"

void ConsoleSessionManager::registerChannel(int stemId, const std::string& name)
{
    channels[stemId] = { stemId, name };
}

bool ConsoleSessionManager::hasChannel(int stemId) const
{
    return channels.find(stemId) != channels.end();
}