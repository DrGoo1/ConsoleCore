#pragma once
#include <string>

enum class ConsoleCorePluginRole
{
    Sender,
    Return,
    Control,
    Monitor
};

struct PluginIdentity
{
    ConsoleCorePluginRole role = ConsoleCorePluginRole::Sender;
    std::string instanceId;
    int stemId = 0;
};