// Bench and offline renderer for physical tape machine profiles.
//
//   tape_machine_bench measure <profile.json> [--ips N] [--rate HZ]
//   tape_machine_bench render  <profile.json> <in.wav> <out.wav> [--ips N] [--drive DB]
//                              [--noise] [--no-flutter] [--linearizer-off]
//
// "measure" prints the model's behaviour beside whatever the profile records as published,
// and exits non-zero if a published limit is missed. "render" processes a WAV file so the
// model can be heard; output is 24-bit PCM, latency-compensated, same length as the input.

#include "../../modules/tape_core/PhysicalTapeMachine.h"
#include "../../modules/tape_core/TapeBench.h"
#include "../../modules/tape_core/TapeSpecJson.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    struct Wav
    {
        int channels = 0;
        int sampleRate = 0;
        std::vector<std::vector<float>> data;  // [channel][frame]
    };

    std::uint32_t le32(const unsigned char* p) { return p[0] | (p[1] << 8) | (p[2] << 16) | (static_cast<std::uint32_t>(p[3]) << 24); }
    std::uint16_t le16(const unsigned char* p) { return static_cast<std::uint16_t>(p[0] | (p[1] << 8)); }

    bool readWav(const std::string& path, Wav& wav, std::string& error)
    {
        std::ifstream f(path, std::ios::binary);
        if (!f) { error = "cannot open " + path; return false; }
        std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        if (bytes.size() < 44 || std::memcmp(bytes.data(), "RIFF", 4) != 0 || std::memcmp(bytes.data() + 8, "WAVE", 4) != 0)
        { error = "not a RIFF/WAVE file"; return false; }
        int format = 0, bits = 0, blockAlign = 0;
        std::size_t pos = 12, dataPos = 0, dataSize = 0;
        while (pos + 8 <= bytes.size())
        {
            const std::uint32_t size = le32(bytes.data() + pos + 4);
            const unsigned char* body = bytes.data() + pos + 8;
            if (pos + 8 + size > bytes.size() && std::memcmp(bytes.data() + pos, "data", 4) != 0)
                break;
            if (std::memcmp(bytes.data() + pos, "fmt ", 4) == 0 && size >= 16)
            {
                format = le16(body);
                wav.channels = le16(body + 2);
                wav.sampleRate = static_cast<int>(le32(body + 4));
                blockAlign = le16(body + 12);
                bits = le16(body + 14);
                if (format == 0xFFFE && size >= 26)
                    format = le16(body + 24);  // WAVE_FORMAT_EXTENSIBLE sub-format
            }
            else if (std::memcmp(bytes.data() + pos, "data", 4) == 0)
            {
                dataPos = pos + 8;
                dataSize = std::min<std::size_t>(size, bytes.size() - dataPos);
                break;
            }
            pos += 8 + size + (size & 1);
        }
        const bool pcm = format == 1 && (bits == 16 || bits == 24 || bits == 32);
        const bool flt = format == 3 && bits == 32;
        if (dataPos == 0 || wav.channels <= 0 || wav.channels > 64 || blockAlign <= 0 || !(pcm || flt))
        { error = "unsupported WAV: need 16/24/32-bit PCM or 32-bit float"; return false; }
        const std::size_t frames = dataSize / static_cast<std::size_t>(blockAlign);
        const int bytesPer = bits / 8;
        wav.data.assign(static_cast<std::size_t>(wav.channels), std::vector<float>(frames));
        for (std::size_t i = 0; i < frames; ++i)
            for (int c = 0; c < wav.channels; ++c)
            {
                const unsigned char* p = bytes.data() + dataPos + i * static_cast<std::size_t>(blockAlign) + static_cast<std::size_t>(c * bytesPer);
                float v = 0.0f;
                if (flt) { std::memcpy(&v, p, 4); }
                else if (bits == 16) v = static_cast<std::int16_t>(le16(p)) / 32768.0f;
                else if (bits == 24) v = static_cast<float>((static_cast<std::int32_t>(p[0] | (p[1] << 8) | (p[2] << 16)) << 8) >> 8) / 8388608.0f;
                else v = static_cast<float>(static_cast<std::int32_t>(le32(p)) / 2147483648.0);
                wav.data[static_cast<std::size_t>(c)][i] = v;
            }
        return true;
    }

    bool writeWav24(const std::string& path, const Wav& wav, std::string& error)
    {
        std::ofstream f(path, std::ios::binary);
        if (!f) { error = "cannot write " + path; return false; }
        const std::uint32_t frames = static_cast<std::uint32_t>(wav.data.empty() ? 0 : wav.data[0].size());
        const std::uint32_t dataSize = frames * static_cast<std::uint32_t>(wav.channels) * 3u;
        auto put32 = [&](std::uint32_t v) { const char b[4] = { char(v), char(v >> 8), char(v >> 16), char(v >> 24) }; f.write(b, 4); };
        auto put16 = [&](std::uint16_t v) { const char b[2] = { char(v), char(v >> 8) }; f.write(b, 2); };
        f.write("RIFF", 4); put32(36 + dataSize); f.write("WAVEfmt ", 8); put32(16); put16(1);
        put16(static_cast<std::uint16_t>(wav.channels)); put32(static_cast<std::uint32_t>(wav.sampleRate));
        put32(static_cast<std::uint32_t>(wav.sampleRate * wav.channels * 3)); put16(static_cast<std::uint16_t>(wav.channels * 3)); put16(24);
        f.write("data", 4); put32(dataSize);
        std::uint32_t clipped = 0;
        for (std::uint32_t i = 0; i < frames; ++i)
            for (int c = 0; c < wav.channels; ++c)
            {
                double v = wav.data[static_cast<std::size_t>(c)][i];
                if (v > 0.9999999 || v < -1.0) { ++clipped; v = v > 0 ? 0.9999999 : -1.0; }
                const std::int32_t s = static_cast<std::int32_t>(v * 8388608.0);
                const char b[3] = { char(s), char(s >> 8), char(s >> 16) };
                f.write(b, 3);
            }
        if (clipped > 0)
            std::cerr << "warning: " << clipped << " samples exceeded full scale and were clipped\n";
        return static_cast<bool>(f);
    }

    const char* option(int argc, char** argv, const char* name)
    {
        for (int i = 1; i + 1 < argc; ++i)
            if (std::strcmp(argv[i], name) == 0)
                return argv[i + 1];
        return nullptr;
    }

    bool hasFlag(int argc, char** argv, const char* name)
    {
        for (int i = 1; i < argc; ++i)
            if (std::strcmp(argv[i], name) == 0)
                return true;
        return false;
    }

    int usage()
    {
        std::cerr << "usage:\n  tape_machine_bench measure <profile.json> [--ips N] [--rate HZ]\n"
                     "  tape_machine_bench render <profile.json> <in.wav> <out.wav> [--ips N] [--drive DB] [--noise] [--no-flutter] [--linearizer-off]\n";
        return 2;
    }
}

int main(int argc, char** argv)
{
    if (argc < 3)
        return usage();
    const std::string command = argv[1];
    tape::MachineSpec spec;
    std::string error;
    if (!tape::loadMachineSpecFile(argv[2], spec, error))
    {
        std::cerr << "profile error: " << error << "\n";
        return 2;
    }
    const auto problems = tape::validateMachineSpec(spec);
    for (const auto& p : problems)
        std::cerr << "profile problem: " << p << "\n";
    if (!problems.empty())
        return 2;

    if (command == "measure")
    {
        const double rate = option(argc, argv, "--rate") ? std::atof(option(argc, argv, "--rate")) : 48000.0;
        std::cout << spec.displayName << "\n  status: " << spec.status << "\n  reference: " << spec.referenceMachine << "\n";
        bool allPass = true;
        for (const auto& speed : spec.speeds)
        {
            if (option(argc, argv, "--ips") && std::abs(std::atof(option(argc, argv, "--ips")) - speed.ips) > 0.01)
                continue;
            tape::PhysicalTapeMachine probe;
            tape::RunSettings run;
            run.ips = speed.ips;
            probe.prepare(rate, 512, 1, spec, run);
            std::printf("\n%.1f ips, %s equalization, %.0f Hz sample rate\n", speed.ips, speed.equalization.c_str(), rate);
            std::printf("  head bump %.0f Hz, short-wavelength corner %.0f Hz, latency %d samples\n", probe.headBumpHz(), probe.recordLossCornerHz(), probe.latencySamples());
            for (const auto& row : tape::bench::report(spec, speed.ips, rate))
            {
                std::printf("  %-52s %-26s %-22s %-10s %s\n", row.name.c_str(), row.measured.c_str(), row.limit.c_str(),
                            row.hasLimit ? (row.pass ? "PASS" : "FAIL") : "-", row.basis.c_str());
                allPass = allPass && (!row.hasLimit || row.pass);
            }
        }
        std::cout << "\nA PASS means the model sits inside a figure taken from documentation. It does not mean the\n"
                     "model matches a real machine: no hardware was measured.\n";
        return allPass ? 0 : 1;
    }

    if (command == "render" && argc >= 5)
    {
        Wav wav;
        if (!readWav(argv[3], wav, error))
        {
            std::cerr << "input error: " << error << "\n";
            return 2;
        }
        tape::RunSettings run;
        run.ips = option(argc, argv, "--ips") ? std::atof(option(argc, argv, "--ips")) : 15.0;
        run.inputDriveDb = option(argc, argv, "--drive") ? std::atof(option(argc, argv, "--drive")) : 0.0;
        run.noiseEnabled = hasFlag(argc, argv, "--noise");
        run.flutterEnabled = !hasFlag(argc, argv, "--no-flutter");
        run.linearizerEnabled = !hasFlag(argc, argv, "--linearizer-off");
        tape::PhysicalTapeMachine machine;
        const int block = 1024;
        if (!machine.prepare(wav.sampleRate, block, wav.channels, spec, run))
        {
            std::cerr << "this profile has no entry for " << run.ips << " ips\n";
            return 2;
        }
        const std::size_t frames = wav.data[0].size();
        const std::size_t latency = static_cast<std::size_t>(machine.latencySamples());
        for (auto& ch : wav.data)
            ch.resize(frames + latency, 0.0f);
        std::vector<float*> pointers(static_cast<std::size_t>(wav.channels));
        for (std::size_t pos = 0; pos < frames + latency; pos += block)
        {
            for (int c = 0; c < wav.channels; ++c)
                pointers[static_cast<std::size_t>(c)] = wav.data[static_cast<std::size_t>(c)].data() + pos;
            machine.process(pointers.data(), wav.channels, static_cast<int>(std::min<std::size_t>(block, frames + latency - pos)));
        }
        for (auto& ch : wav.data)
            ch.erase(ch.begin(), ch.begin() + static_cast<std::ptrdiff_t>(latency));
        if (!writeWav24(argv[4], wav, error))
        {
            std::cerr << "output error: " << error << "\n";
            return 2;
        }
        std::cout << "rendered " << frames << " frames, " << wav.channels << " channel(s), through " << spec.displayName << " at " << run.ips
                  << " ips\nstatus: " << spec.status << "\n";
        return 0;
    }
    return usage();
}
