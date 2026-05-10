#include "OscSyncManager.h"

bool OscSyncManager::initialize(int localPort)
{
    port = localPort;
    return true;
}

void OscSyncManager::sendTransportState(bool playing, double bpm)
{
    // Phase 4 scaffold only
}

void OscSyncManager::receiveMessages()
{
    // Future OSC receive handling
}