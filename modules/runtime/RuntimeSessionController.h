#pragma once
#include <string>
#include <vector>
#include <optional>

struct RuntimeSnapshot
{
    std::string name;
    float drive = 0.25f;
    float railStressAmount = 0.35f;
    float transformerWeight = 0.25f;
    float crosstalkAmount = 0.08f;
    float tapeMemoryAmount = 0.15f;
};

class RuntimeSessionController
{
public:
    bool loadProfile(const std::string& path);
    bool saveProfile(const std::string& path) const;

    void addSnapshot(const RuntimeSnapshot& snapshot);
    bool recallSnapshot(const std::string& name);

    std::optional<RuntimeSnapshot> getCurrentSnapshot() const;

private:
    std::string activeProfilePath;
    std::vector<RuntimeSnapshot> snapshots;
    std::optional<RuntimeSnapshot> currentSnapshot;
};