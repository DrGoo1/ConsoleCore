#include "AnalogPhysicsCore.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

static float runCase(int blockSize)
{
    constexpr double fs = 96000.0;
    constexpr int total = 96000;
    AnalogPhysicsCore core;
    core.prepare(fs, blockSize);
    AnalogPhysicsParameters params;
    core.setParameters(params);
    std::vector<float> l(blockSize), r(blockSize);
    int done = 0;
    while (done < total)
    {
        const int n = std::min(blockSize, total - done);
        for (int i = 0; i < n; ++i)
        {
            const double t = double(done + i) / fs;
            l[i] = 0.28f * std::sin(2.0 * 3.14159265358979323846 * 55.0 * t);
            r[i] = l[i];
        }
        core.processBlock(l.data(), r.data(), n, 24);
        done += n;
    }
    return core.getTelemetry().railStress;
}

int main()
{
    const float ref = runCase(64);
    for (int blockSize : {256, 1024, 4096})
    {
        const float value = runCase(blockSize);
        if (std::abs(value - ref) > 1.0e-6f)
        {
            std::cerr << "FAIL block-size variance: " << ref << " vs " << value << " at " << blockSize << "\n";
            return 1;
        }
    }
    std::cout << "PASS rail/shared-state block-size invariant: " << ref << "\n";
    return 0;
}
