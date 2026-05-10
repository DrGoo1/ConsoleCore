#include "ConsoleCoreEngine.h"
#include "TopologyGraph.h"
#include "AnalogPhysicsCore.h"
#include "ProfileValidator.h"
#include "RuntimeSessionController.h"
#include "LatencyCalibration.h"
#include <iostream>
#include <vector>
#include <cmath>

int main()
{
    TopologyGraph graph;
    graph.addNode(ConsoleNode{ "channel_1", 2.0f, 0.0f });
    graph.update(0.5f);
    if (graph.getNodes().empty() || graph.getNodes()[0].interaction != 1.0f)
        return 1;

    {
        constexpr double fs = 48000.0;
        constexpr int n = 1024;

        std::vector<float> l(n), r(n);
        for (int i = 0; i < n; ++i)
        {
            const float tone = 0.2f * std::sin(2.0 * 3.141592653589793 * 440.0 * static_cast<double>(i) / fs);
            const float impulse = (i == 64) ? 0.8f : 0.0f;
            l[i] = tone + impulse;
            r[i] = 0.9f * tone - 0.5f * impulse;
        }

        AnalogPhysicsCore core;
        core.prepare(fs, n);

        AnalogPhysicsParameters params;
        params.drive = 0.25f;
        params.railStressAmount = 0.45f;
        params.transformerWeight = 0.35f;
        params.crosstalkAmount = 0.10f;
        params.tapeMemoryAmount = 0.15f;
        core.setParameters(params);

        core.processBlock(l.data(), r.data(), n, 8);
        const auto& t = core.getTelemetry();

        if (!std::isfinite(t.railVoltage) || !std::isfinite(t.railStress) || !std::isfinite(t.peakOutput))
            return 1;

        for (int i = 0; i < n; ++i)
        {
            if (!std::isfinite(l[i]) || !std::isfinite(r[i]))
                return 1;
            if (std::abs(l[i]) > 4.0f || std::abs(r[i]) > 4.0f)
                return 1;
        }
    }

    {
        AnalogPhysicsParameters params;
        params.drive = 0.20f;
        params.railStressAmount = 0.30f;
        params.transformerWeight = 0.25f;
        auto report = ProfileValidator::validate(params);
        if (!report.stable)
            return 1;
    }

    {
        RuntimeSessionController runtime;
        RuntimeSnapshot a;
        a.name = "A";
        a.drive = 0.10f;
        RuntimeSnapshot b;
        b.name = "B";
        b.drive = 0.30f;
        runtime.addSnapshot(a);
        runtime.addSnapshot(b);
        if (!runtime.recallSnapshot("B"))
            return 1;
        auto current = runtime.getCurrentSnapshot();
        if (!current.has_value() || current->name != "B")
            return 1;

        LatencyCalibration cal;
        cal.measuredRoundtripSamples = 100;
        cal.manualOffsetSamples = -12;
        if (cal.compensatedLatencySamples() != 88)
            return 1;
    }

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
