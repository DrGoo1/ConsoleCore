#pragma once
#include <string>

class OscSyncManager
{
public:
    bool initialize(int localPort);
    void sendTransportState(bool playing, double bpm);
    void receiveMessages();

private:
    int port = 0;
};