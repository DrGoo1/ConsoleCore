#include "RemoteTelemetryAggregator.h"

void RemoteTelemetryAggregator::push(const NodeTelemetry& telemetry)
{
    telemetryFrames.push_back(telemetry);
}

const std::vector<NodeTelemetry>& RemoteTelemetryAggregator::all() const
{
    return telemetryFrames;
}