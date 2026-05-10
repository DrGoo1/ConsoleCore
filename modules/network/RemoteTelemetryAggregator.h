#pragma once
#include "NodeTelemetry.h"
#include <vector>

class RemoteTelemetryAggregator
{
public:
    void push(const NodeTelemetry& telemetry);
    const std::vector<NodeTelemetry>& all() const;

private:
    std::vector<NodeTelemetry> telemetryFrames;
};