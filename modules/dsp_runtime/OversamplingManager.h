#pragma once

enum class OversamplingMode
{
    Off,
    X2,
    X4,
    X8
};

class OversamplingManager
{
public:
    void setMode(OversamplingMode m);
    OversamplingMode getMode() const;

    int factor() const;

private:
    OversamplingMode mode = OversamplingMode::Off;
};