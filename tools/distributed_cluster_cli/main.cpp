#include "../../modules/network/ClusterTopology.h"
#include "../../modules/network/CapabilityRegistry.h"
#include "../../modules/network/RemoteTelemetryAggregator.h"
#include <iostream>

int main()
{
    ClusterTopology topology;

    topology.addNode({
        "node-local",
        "MacStudio",
        NodeRole::Local,
        12,
        1,
        true
    });

    topology.addNode({
        "node-render",
        "RenderServer",
        NodeRole::Render,
        32,
        2,
        true
    });

    CapabilityRegistry caps;
    caps.addCapability({"MetalAcceleration"});
    caps.addCapability({"OfflineRender"});

    RemoteTelemetryAggregator telemetry;
    telemetry.push({18.2f, 8.4f, 1.2f, 2});

    std::cout << "ConsoleCore Distributed Cluster CLI\n";
    std::cout << "Cluster Nodes: " << topology.nodes().size() << "\n";
    std::cout << "Capabilities: " << caps.capabilities().size() << "\n";
    std::cout << "Telemetry Frames: " << telemetry.all().size() << "\n";

    return 0;
}