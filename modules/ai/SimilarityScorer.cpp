#include "SimilarityScorer.h"
#include <cmath>
#include <algorithm>

float SimilarityScorer::cosineSimilarity(const FeatureVector& a, const FeatureVector& b)
{
    const size_t n = std::min(a.embedding.size(), b.embedding.size());

    if (n == 0)
        return 0.0f;

    double dot = 0.0;
    double magA = 0.0;
    double magB = 0.0;

    for (size_t i = 0; i < n; ++i)
    {
        dot += a.embedding[i] * b.embedding[i];
        magA += a.embedding[i] * a.embedding[i];
        magB += b.embedding[i] * b.embedding[i];
    }

    if (magA <= 1.0e-12 || magB <= 1.0e-12)
        return 0.0f;

    return static_cast<float>(dot / (std::sqrt(magA) * std::sqrt(magB)));
}