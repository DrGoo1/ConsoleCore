#include "OversamplingManager.h"

void OversamplingManager::setMode(OversamplingMode m)
{
    mode = m;
}

OversamplingMode OversamplingManager::getMode() const
{
    return mode;
}

int OversamplingManager::factor() const
{
    switch (mode)
    {
        case OversamplingMode::X2: return 2;
        case OversamplingMode::X4: return 4;
        case OversamplingMode::X8: return 8;
        default: return 1;
    }
}