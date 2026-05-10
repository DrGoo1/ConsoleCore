#pragma once
#include <string>

enum class HardwareType
{
    Console,
    TapeMachine,
    SummingMixer,
    Compressor,
    Equalizer,
    MicPreamp,
    Converter,
    Unknown
};

struct HardwareDevice
{
    std::string manufacturer;
    std::string model;
    HardwareType type = HardwareType::Unknown;
    std::string notes;
};