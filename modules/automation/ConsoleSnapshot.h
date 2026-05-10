#pragma once
#include <string>
#include <map>

struct ConsoleSnapshot
{
    std::string name;
    std::map<std::string, float> parameters;
};