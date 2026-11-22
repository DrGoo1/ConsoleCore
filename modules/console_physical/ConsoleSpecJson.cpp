#include "ConsoleSpecJson.h"
#include "../tape_core/MiniJson.h"

#include <fstream>
#include <sstream>

namespace console
{
    using namespace tape::json;

    bool parseStageSpec(const std::string& jsonText, StageSpec& out, std::string& error)
    {
        try
        {
            Parser parser(jsonText);
            const Value root = parser.parseDocument();
            if (root.kind != Value::Kind::Object)
                throw std::runtime_error("profile must be a JSON object");
            if (text(root, "schema", true) != "consolecore.stage.v1")
                throw std::runtime_error("schema must be 'consolecore.stage.v1'");

            StageSpec s;
            s.id = text(root, "id", true);
            s.displayName = text(root, "displayName", true);
            s.referenceModule = text(root, "referenceModule");
            s.status = text(root, "status", true);
            s.referenceDbfs = number(root, "referenceDbfs", s.referenceDbfs);
            s.nominalLevelDbu = number(root, "nominalLevelDbu", s.nominalLevelDbu);

            const Value& amp = *object(root, "amplifier");
            s.thdPercentAtCal = number(amp, "thdPercentAtCal", 0, true);
            s.thdCalLevelDbu = number(amp, "thdCalLevelDbu", 0, true);
            s.maxOutputDbu = number(amp, "maxOutputDbu", 0, true);
            s.thdPercentAtMaxOutput = number(amp, "thdPercentAtMaxOutput", s.thdPercentAtMaxOutput);
            s.clipCeilingDbu = number(amp, "clipCeilingDbu", 0, true);
            s.hfCornerHz = number(amp, "hfCornerHz", 0, true);
            s.noiseDbu = number(amp, "noiseDbu", s.noiseDbu);

            const Value& iron = *object(root, "transformers");
            s.inputTransformer = flag(iron, "input", false);
            s.outputTransformer = flag(iron, "output", false);
            s.transformerHd3PercentAtCal = number(iron, "hd3PercentAtCal", s.transformerHd3PercentAtCal);
            s.transformerCalHz = number(iron, "calHz", s.transformerCalHz);
            s.lfCornerHz = number(iron, "lfCornerHz", s.lfCornerHz);

            if (const Value* pub = root.find("published"); pub != nullptr && pub->kind == Value::Kind::Object)
            {
                s.publishedThdMaxPercent = number(*pub, "thdMaxPercent", 0.0);
                s.publishedThdLoHz = number(*pub, "thdLoHz", s.publishedThdLoHz);
                s.publishedThdHiHz = number(*pub, "thdHiHz", s.publishedThdHiHz);
                if (const Value* bands = pub->find("response"); bands != nullptr && bands->kind == Value::Kind::Array)
                    for (const auto& b : bands->items)
                    {
                        ResponseBand band;
                        band.loHz = number(b, "loHz", 0, true);
                        band.hiHz = number(b, "hiHz", 0, true);
                        band.plusDb = number(b, "plusDb", 0, true);
                        band.minusDb = number(b, "minusDb", 0, true);
                        s.publishedResponse.push_back(band);
                    }
            }

            if (const Value* evidence = root.find("evidence"); evidence != nullptr && evidence->kind == Value::Kind::Object)
                for (const auto& m : evidence->members)
                    if (m.second.kind == Value::Kind::String)
                        s.evidence[m.first] = m.second.text;

            out = std::move(s);
            error.clear();
            return true;
        }
        catch (const std::exception& e)
        {
            error = e.what();
            return false;
        }
    }

    bool loadStageSpecFile(const std::string& path, StageSpec& out, std::string& error)
    {
        std::ifstream file(path);
        if (!file)
        {
            error = "cannot open profile '" + path + "'";
            return false;
        }
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return parseStageSpec(buffer.str(), out, error);
    }

    std::vector<std::string> validateStageSpec(const StageSpec& s)
    {
        std::vector<std::string> problems;
        auto range = [&](const std::string& name, double v, double lo, double hi)
        {
            if (!(v >= lo && v <= hi))
                problems.push_back(name + " = " + std::to_string(v) + " is outside " + std::to_string(lo) + ".." + std::to_string(hi));
        };
        range("referenceDbfs", s.referenceDbfs, -30.0, -6.0);
        range("nominalLevelDbu", s.nominalLevelDbu, -10.0, 8.0);
        range("amplifier.thdPercentAtCal", s.thdPercentAtCal, 0.001, 2.0);
        range("amplifier.thdCalLevelDbu", s.thdCalLevelDbu, s.nominalLevelDbu, 30.0);
        range("amplifier.maxOutputDbu", s.maxOutputDbu, s.thdCalLevelDbu + 1.0, 36.0);
        range("amplifier.thdPercentAtMaxOutput", s.thdPercentAtMaxOutput, s.thdPercentAtCal, 10.0);
        range("amplifier.clipCeilingDbu", s.clipCeilingDbu, s.maxOutputDbu + 0.5, 40.0);
        range("amplifier.hfCornerHz", s.hfCornerHz, 15000.0, 300000.0);
        range("amplifier.noiseDbu", s.noiseDbu, -120.0, -50.0);
        range("transformers.hd3PercentAtCal", s.transformerHd3PercentAtCal, 0.0, 2.0);
        range("transformers.calHz", s.transformerCalHz, 10.0, 200.0);
        range("transformers.lfCornerHz", s.lfCornerHz, 0.5, 40.0);

        for (const char* key : { "referenceLevel", "amplifier.thd", "amplifier.maxOutput", "amplifier.clipCeiling", "amplifier.hfCorner",
                                 "amplifier.noise", "amplifier.harmonicBalance", "transformers.fitted", "transformers.hd3", "transformers.lfCorner" })
        {
            const auto it = s.evidence.find(key);
            if (it == s.evidence.end())
            {
                problems.push_back(std::string("no evidence entry for ") + key);
                continue;
            }
            const std::string& e = it->second;
            if (!(e.rfind("published:", 0) == 0 || e.rfind("standard:", 0) == 0 || e.rfind("derived:", 0) == 0 || e.rfind("assumed:", 0) == 0))
                problems.push_back(std::string("evidence for ") + key + " must start with published:, standard:, derived: or assumed:");
        }
        return problems;
    }
}
