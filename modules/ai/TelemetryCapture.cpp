#include "TelemetryCapture.h"

void TelemetryCapture::push(const TelemetryPoint& p)
{
    history.push_back(p);
}

const std::vector<TelemetryPoint>& TelemetryCapture::points() const
{
    return history;
}