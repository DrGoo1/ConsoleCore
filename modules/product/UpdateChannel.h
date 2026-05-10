#pragma once
#include <string>

enum class UpdateChannelType
{
    Stable,
    Beta,
    Experimental
};

struct UpdateChannel
{
    UpdateChannelType type = UpdateChannelType::Stable;
    std::string updateUrl;
};