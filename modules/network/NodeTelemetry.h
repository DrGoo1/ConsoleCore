#pragma once

struct NodeTelemetry
{
    float cpuLoad = 0.0f;
    float memoryUsageGb = 0.0f;
    float networkLatencyMs = 0.0f;
    int activeTasks = 0;
};