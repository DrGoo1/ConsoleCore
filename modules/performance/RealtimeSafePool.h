#pragma once
#include <vector>
#include <cstddef>

class RealtimeSafePool
{
public:
    void initialize(size_t bytes);
    void* allocate(size_t bytes);
    void reset();

private:
    std::vector<char> buffer;
    size_t offset = 0;
};