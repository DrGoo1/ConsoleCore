#pragma once

struct RealtimeHardwareTelemetry
{
    float estimatedNoiseFloorDb = -120.0f;
    float estimatedHeadroomDb = 24.0f;
    float railStress = 0.0f;
    float thermalEstimate = 0.0f;
};