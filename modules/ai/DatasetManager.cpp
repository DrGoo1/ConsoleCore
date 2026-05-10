#include "DatasetManager.h"

void DatasetManager::addEntry(const DatasetEntry& entry)
{
    dataset.push_back(entry);
}

const std::vector<DatasetEntry>& DatasetManager::entries() const
{
    return dataset;
}