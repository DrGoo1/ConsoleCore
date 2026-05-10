#include "RuntimeSessionController.h"
#include <fstream>

bool RuntimeSessionController::loadProfile(const std::string& path)
{
    std::ifstream in(path);
    if (!in) return false;
    activeProfilePath = path;
    return true;
}

bool RuntimeSessionController::saveProfile(const std::string& path) const
{
    std::ofstream out(path);
    if (!out) return false;
    out << "{\n";
    out << "  \"activeProfilePath\": \"" << activeProfilePath << "\",\n";
    out << "  \"snapshotCount\": " << snapshots.size() << "\n";
    out << "}\n";
    return true;
}

void RuntimeSessionController::addSnapshot(const RuntimeSnapshot& snapshot)
{
    snapshots.push_back(snapshot);
    if (!currentSnapshot.has_value())
        currentSnapshot = snapshot;
}

bool RuntimeSessionController::recallSnapshot(const std::string& name)
{
    for (const auto& s : snapshots)
    {
        if (s.name == name)
        {
            currentSnapshot = s;
            return true;
        }
    }
    return false;
}

std::optional<RuntimeSnapshot> RuntimeSessionController::getCurrentSnapshot() const
{
    return currentSnapshot;
}