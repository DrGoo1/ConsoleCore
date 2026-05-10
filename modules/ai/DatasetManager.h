#pragma once
#include <string>
#include <vector>

struct DatasetEntry
{
    std::string captureName;
    std::string featurePath;
    std::string profilePath;
};

class DatasetManager
{
public:
    void addEntry(const DatasetEntry& entry);
    const std::vector<DatasetEntry>& entries() const;

private:
    std::vector<DatasetEntry> dataset;
};