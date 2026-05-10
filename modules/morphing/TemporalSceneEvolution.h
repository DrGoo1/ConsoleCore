#pragma once
#include <vector>

struct SceneEvolutionPoint
{
    float timeSeconds = 0.0f;
    float morphAmount = 0.0f;
};

class TemporalSceneEvolution
{
public:
    void addPoint(const SceneEvolutionPoint& point);
    const std::vector<SceneEvolutionPoint>& points() const;

private:
    std::vector<SceneEvolutionPoint> evolution;
};