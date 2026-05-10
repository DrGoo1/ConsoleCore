#include "ConsoleCoreEngine.h"
#include <iostream>

int main()
{
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
