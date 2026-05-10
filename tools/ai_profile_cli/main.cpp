#include "../../modules/ai/SimilarityScorer.h"
#include "../../modules/ai/AdaptiveConsoleMatcher.h"
#include "../../modules/ai/DatasetManager.h"
#include "../../modules/ai/TelemetryCapture.h"
#include <iostream>

int main()
{
    FeatureVector a;
    a.sourceName = "MixA";
    a.embedding = {0.1f, 0.3f, 0.5f, 0.2f};

    FeatureVector b;
    b.sourceName = "MixB";
    b.embedding = {0.12f, 0.28f, 0.48f, 0.25f};

    const float similarity = SimilarityScorer::cosineSimilarity(a, b);

    SessionFingerprint session;
    session.averageRailStress = 0.62f;

    AdaptiveConsoleMatcher matcher;
    auto result = matcher.suggestProfile(session);

    DatasetManager dataset;
    dataset.addEntry({"ExampleCapture", "features.json", "profile.json"});

    TelemetryCapture telemetry;
    telemetry.push({0.42f, 0.88f, 0.91f});

    std::cout << "ConsoleCore AI Profile CLI\n";
    std::cout << "Similarity: " << similarity << "\n";
    std::cout << "Suggested Profile: " << result.profileName << "\n";
    std::cout << "Dataset Entries: " << dataset.entries().size() << "\n";
    std::cout << "Telemetry Samples: " << telemetry.points().size() << "\n";

    return 0;
}