#pragma once

struct TapeSharedState
{
    float tapeBiasStress = 0.0f;
    float lowFrequencySaturation = 0.0f;
    float tapeCompression = 0.0f;
    float transportInstability = 0.0f;
    float adjacentTrackCoupling = 0.0f;
    float printTapeStress = 0.0f;

    void reset()
    {
        tapeBiasStress = 0.0f;
        lowFrequencySaturation = 0.0f;
        tapeCompression = 0.0f;
        transportInstability = 0.0f;
        adjacentTrackCoupling = 0.0f;
        printTapeStress = 0.0f;
    }
};
