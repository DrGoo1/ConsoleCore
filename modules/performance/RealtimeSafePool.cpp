#include "RealtimeSafePool.h"

void RealtimeSafePool::initialize(size_t bytes)
{
    buffer.resize(bytes);
    offset = 0;
}

void* RealtimeSafePool::allocate(size_t bytes)
{
    if (offset + bytes > buffer.size())
        return nullptr;

    void* ptr = buffer.data() + offset;
    offset += bytes;
    return ptr;
}

void RealtimeSafePool::reset()
{
    offset = 0;
}