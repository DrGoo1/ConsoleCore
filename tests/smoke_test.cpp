#include "ConsoleCoreEngine.h"
#include "TopologyGraph.h"
#include <iostream>

int main()
{
    TopologyGraph graph;
    graph.addNode(ConsoleNode{ "channel_1", 2.0f, 0.0f });
    graph.update(0.5f);
    if (graph.getNodes().empty() || graph.getNodes()[0].interaction != 1.0f)
        return 1;

    consolecore::ConsoleCoreEngine engine;
    engine.prepare(48000.0, 512, 2);
    juce::AudioBuffer<float> buffer(2, 512);
    buffer.clear();
    engine.process(buffer);
    if (engine.getRailState().getActiveChannels() != 0)
        return 1;
    std::cout << "ConsoleCore smoke test passed\n";
    return 0;
}
