#pragma once
#include <map>
#include <string>

struct ChannelAssignment
{
    int stemId;
    std::string name;
};

class ConsoleSessionManager
{
public:
    void registerChannel(int stemId, const std::string& name);
    bool hasChannel(int stemId) const;

private:
    std::map<int, ChannelAssignment> channels;
};