#include "../../modules/model_capture/TestSignalGenerator.h"
#include "../../modules/model_capture/FeatureExtractor.h"
#include "../../modules/model_capture/ModelFitter.h"
#include "../../modules/model_capture/ProfileValidator.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

static void writeRawFloat(const std::string& path, const std::vector<float>& data)
{
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size() * sizeof(float)));
}

int main(int argc, char** argv)
{
    const double fs = 48000.0;

    if (argc < 2)
    {
        std::cout << "ConsoleCore Model Capture CLI\n";
        std::cout << "Usage:\n";
        std::cout << "  generate <type> <out.raw>\n";
        std::cout << "  validate\n";
        return 0;
    }

    const std::string cmd = argv[1];

    if (cmd == "generate")
    {
        const std::string type = argc > 2 ? argv[2] : "sweep";
        const std::string outPath = argc > 3 ? argv[3] : "test_signal.raw";

        TestSignalType signal = TestSignalType::LogSweep;
        if (type == "sine") signal = TestSignalType::Sine1k;
        else if (type == "impulse") signal = TestSignalType::Impulse;
        else if (type == "step") signal = TestSignalType::Step;
        else if (type == "burst") signal = TestSignalType::BurstTrain;
        else if (type == "noise") signal = TestSignalType::PinkishNoise;
        else if (type == "multitone") signal = TestSignalType::Multitone;

        auto data = TestSignalGenerator::generate(signal, fs, 5.0, 0.5f);
        writeRawFloat(outPath, data);
        std::cout << "Wrote raw float test signal: " << outPath << "\n";
        return 0;
    }

    if (cmd == "validate")
    {
        AnalogPhysicsParameters p;
        p.drive = 0.28f;
        p.railStressAmount = 0.42f;
        p.transformerWeight = 0.40f;

        auto report = ProfileValidator::validate(p);
        std::cout << report.message << "\n";
        std::cout << "Peak: " << report.peakOutput << "\n";
        std::cout << "Rail Stress: " << report.railStress << "\n";
        std::cout << "Headroom Gain: " << report.headroomGain << "\n";
        return report.stable ? 0 : 1;
    }

    std::cout << "Unknown command.\n";
    return 1;
}