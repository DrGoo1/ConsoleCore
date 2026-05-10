#pragma once
#include <string>

enum class RenderTaskState
{
    Queued,
    Running,
    Completed,
    Failed
};

struct DistributedRenderTask
{
    std::string taskId;
    std::string sourceSession;
    std::string destinationPath;

    RenderTaskState state = RenderTaskState::Queued;
};