#include "LockFreeTelemetry.h"

void LockFreeTelemetry::push(const TelemetryFrame& frame)
{
    railStress.store(frame.railStress);
    peak.store(frame.peak);
    cpuLoad.store(frame.cpuLoad);
}

TelemetryFrame LockFreeTelemetry::latest() const
{
    TelemetryFrame f;
    f.railStress = railStress.load();
    f.peak = peak.load();
    f.cpuLoad = cpuLoad.load();
    return f;
}