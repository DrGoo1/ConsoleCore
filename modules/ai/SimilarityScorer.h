#pragma once
#include "FeatureVector.h"

class SimilarityScorer
{
public:
    static float cosineSimilarity(const FeatureVector& a, const FeatureVector& b);
};