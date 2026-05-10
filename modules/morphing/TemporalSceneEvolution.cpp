#include "TemporalSceneEvolution.h"

void TemporalSceneEvolution::addPoint(const SceneEvolutionPoint& point)
{
    evolution.push_back(point);
}

const std::vector<SceneEvolutionPoint>& TemporalSceneEvolution::points() const
{
    return evolution;
}