#pragma once
#include <string>

struct MidiControlMapping
{
    int midiChannel = 1;
    int ccNumber = 0;
    std::string targetParameter;
};