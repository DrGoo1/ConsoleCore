#include "TestSignalGenerator.h"
#include <cmath>
#include <algorithm>

static float pseudoRandom01(int n)
{
    unsigned int x = static_cast<unsigned int>(n * 1103515245u + 12345u);
    x = (x / 65536u) % 32768u;
    return static_cast<float>(x) / 32768.0f;
}

std::vector<float> TestSignalGenerator::generate(TestSignalType type, double sampleRate, double seconds, float amplitude)
{
    const int n = std::max(1, static_cast<int>(sampleRate * seconds));
    std::vector<float> out(n, 0.0f);

    const double twoPi = 6.2831853071795864769;

    switch (type)
    {
        case TestSignalType::Sine1k:
            for (int i = 0; i < n; ++i)
                out[i] = amplitude * std::sin(twoPi * 1000.0 * i / sampleRate);
            break;

        case TestSignalType::Impulse:
            out[0] = amplitude;
            break;

        case TestSignalType::Step:
            for (int i = n / 8; i < n; ++i)
                out[i] = amplitude;
            break;

        case TestSignalType::BurstTrain:
            for (int i = 0; i < n; ++i)
            {
                const int period = static_cast<int>(sampleRate * 0.25);
                const int pos = i % std::max(1, period);
                if (pos < static_cast<int>(sampleRate * 0.02))
                    out[i] = amplitude * std::sin(twoPi * 100.0 * i / sampleRate);
            }
            break;

        case TestSignalType::PinkishNoise:
        {
            float lp = 0.0f;
            for (int i = 0; i < n; ++i)
            {
                const float white = 2.0f * pseudoRandom01(i) - 1.0f;
                lp = 0.985f * lp + 0.015f * white;
                out[i] = amplitude * lp * 3.0f;
            }
            break;
        }

        case TestSignalType::Multitone:
        {
            const double tones[] = { 60.0, 110.0, 220.0, 440.0, 880.0, 1760.0, 3520.0, 7040.0 };
            for (int i = 0; i < n; ++i)
            {
                double v = 0.0;
                for (double f : tones)
                    v += std::sin(twoPi * f * i / sampleRate);
                out[i] = amplitude * static_cast<float>(v / 8.0);
            }
            break;
        }

        case TestSignalType::LogSweep:
        default:
        {
            const double f0 = 20.0;
            const double f1 = 20000.0;
            const double T = std::max(0.001, seconds);
            const double K = T / std::log(f1 / f0);
            const double L = f0 * K;
            for (int i = 0; i < n; ++i)
            {
                const double t = i / sampleRate;
                const double phase = twoPi * L * (std::exp(t / K) - 1.0);
                out[i] = amplitude * static_cast<float>(std::sin(phase));
            }
            break;
        }
    }

    return out;
}