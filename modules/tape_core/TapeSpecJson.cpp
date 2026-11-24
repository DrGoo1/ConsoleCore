#include "TapeSpecJson.h"
#include "MiniJson.h"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace tape
{
    using namespace json;

    bool parseMachineSpec(const std::string& jsonText, MachineSpec& out, std::string& error)
    {
        try
        {
            Parser parser(jsonText);
            const Value root = parser.parseDocument();
            if (root.kind != Value::Kind::Object)
                throw std::runtime_error("profile must be a JSON object");
            if (text(root, "schema", true) != "tapecore.machine.v2")
                throw std::runtime_error("schema must be 'tapecore.machine.v2'");

            MachineSpec spec;
            spec.id = text(root, "id", true);
            spec.displayName = text(root, "displayName", true);
            spec.referenceMachine = text(root, "referenceMachine");
            spec.status = text(root, "status", true);
            spec.referenceDbfs = number(root, "referenceDbfs", spec.referenceDbfs);

            const Value& format = *object(root, "format");
            spec.tracks = static_cast<int>(number(format, "tracks", 0, true));
            spec.tapeWidthInches = number(format, "tapeWidthInches", 0, true);
            spec.trackWidthInches = number(format, "trackWidthInches", 0, true);

            const Value& tapeObj = *object(root, "tape");
            spec.coatingThicknessMicrons = number(tapeObj, "coatingThicknessMicrons", 0, true);
            spec.hd3PercentAtCal = number(tapeObj, "hd3PercentAtCal", 0, true);
            spec.hd3CalLevelDb = number(tapeObj, "hd3CalLevelDb", 0, true);
            spec.publishedHd3PercentAtOperatingLevel = number(tapeObj, "hd3PercentAtOperatingLevel", 0.0);
            spec.hasLinearizer = flag(tapeObj, "hasLinearizer", false);
            spec.linearizerHd3Percent = number(tapeObj, "linearizerHd3Percent", spec.linearizerHd3Percent);
            spec.biasFrequencyHz = number(tapeObj, "biasFrequencyHz", 0.0);
            spec.overbiasDb = number(tapeObj, "overbiasDb", 0.0);
            spec.overbiasReferenceHz = number(tapeObj, "overbiasReferenceHz", spec.overbiasReferenceHz);
            spec.biasRangeToNominalDb = number(tapeObj, "biasRangeToNominalDb", spec.biasRangeToNominalDb);

            const Value& head = *object(root, "head");
            spec.headBumpWavelengthInches = number(head, "headBumpWavelengthInches", 0, true);
            spec.headBumpGainDb = number(head, "headBumpGainDb", 0, true);

            const Value& electronics = *object(root, "electronics");
            spec.inputTransformer = flag(electronics, "inputTransformer", false);
            spec.outputTransformer = flag(electronics, "outputTransformer", false);
            spec.transformerHd3Percent = number(electronics, "transformerHd3Percent", spec.transformerHd3Percent);
            spec.transformerCalHz = number(electronics, "transformerCalHz", spec.transformerCalHz);
            spec.maxOutputDb = number(electronics, "maxOutputDb", 0, true);
            spec.electronicsHighpassHz = number(electronics, "highpassHz", 0, true);

            if (const Value* transport = root.find("transport"); transport != nullptr && transport->kind == Value::Kind::Object)
                if (const Value* comps = transport->find("flutterComponents"); comps != nullptr && comps->kind == Value::Kind::Array)
                    for (const auto& item : comps->items)
                    {
                        FlutterComponent c;
                        c.hzAt15Ips = number(item, "hzAt15Ips", 0, true);
                        c.weight = number(item, "weight", 1.0);
                        spec.flutterComponents.push_back(c);
                    }

            const Value* speeds = root.find("speeds");
            if (speeds == nullptr || speeds->kind != Value::Kind::Array || speeds->items.empty())
                throw std::runtime_error("profile needs a non-empty 'speeds' array");
            for (const auto& item : speeds->items)
            {
                SpeedSpec s;
                s.ips = number(item, "ips", 0, true);
                s.equalization = text(item, "equalization", true);
                s.hfTimeConstantUs = number(item, "hfTimeConstantUs", 0, true);
                s.lfTimeConstantUs = number(item, "lfTimeConstantUs", 0.0);
                s.playbackHfCornerHz = number(item, "playbackHfCornerHz", 0, true);
                s.flutterLowBandRmsPercent = number(item, "flutterLowBandRmsPercent", 0.0);
                s.scrapeFlutterRmsPercent = number(item, "scrapeFlutterRmsPercent", 0.0);
                s.snrDb = number(item, "snrDb", s.snrDb);
                s.referenceHz = number(item, "referenceHz", s.referenceHz);
                s.publishedFlutterMaxPercent = number(item, "publishedFlutterMaxPercent", 0.0);
                if (const Value* bands = item.find("publishedResponse"); bands != nullptr && bands->kind == Value::Kind::Array)
                    for (const auto& b : bands->items)
                    {
                        ResponseBand band;
                        band.loHz = number(b, "loHz", 0, true);
                        band.hiHz = number(b, "hiHz", 0, true);
                        band.plusDb = number(b, "plusDb", 0, true);
                        band.minusDb = number(b, "minusDb", 0, true);
                        s.publishedResponse.push_back(band);
                    }
                spec.speeds.push_back(s);
            }

            if (const Value* evidence = root.find("evidence"); evidence != nullptr && evidence->kind == Value::Kind::Object)
                for (const auto& m : evidence->members)
                    if (m.second.kind == Value::Kind::String)
                        spec.evidence[m.first] = m.second.text;

            out = std::move(spec);
            error.clear();
            return true;
        }
        catch (const std::exception& e)
        {
            error = e.what();
            return false;
        }
    }

    bool loadMachineSpecFile(const std::string& path, MachineSpec& out, std::string& error)
    {
        std::ifstream file(path);
        if (!file)
        {
            error = "cannot open profile '" + path + "'";
            return false;
        }
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return parseMachineSpec(buffer.str(), out, error);
    }

    std::vector<std::string> validateMachineSpec(const MachineSpec& spec)
    {
        std::vector<std::string> problems;
        auto range = [&](const std::string& name, double v, double lo, double hi)
        {
            if (!(v >= lo && v <= hi))
                problems.push_back(name + " = " + std::to_string(v) + " is outside " + std::to_string(lo) + ".." + std::to_string(hi));
        };
        range("format.tracks", spec.tracks, 1, 48);
        range("format.trackWidthInches", spec.trackWidthInches, 0.01, 2.0);
        range("referenceDbfs", spec.referenceDbfs, -30.0, -6.0);
        range("tape.coatingThicknessMicrons", spec.coatingThicknessMicrons, 3.0, 30.0);
        range("tape.hd3PercentAtCal", spec.hd3PercentAtCal, 0.1, 10.0);
        range("tape.hd3CalLevelDb", spec.hd3CalLevelDb, 0.0, 20.0);
        range("tape.overbiasDb", spec.overbiasDb, 0.0, 8.0);
        range("tape.overbiasReferenceHz", spec.overbiasReferenceHz, 1000.0, 20000.0);
        range("tape.biasRangeToNominalDb", spec.biasRangeToNominalDb, 1.0, 6.0);
        range("head.headBumpWavelengthInches", spec.headBumpWavelengthInches, 0.05, 1.5);
        range("head.headBumpGainDb", spec.headBumpGainDb, 0.0, 4.0);
        range("electronics.maxOutputDb", spec.maxOutputDb, 10.0, 36.0);
        range("electronics.highpassHz", spec.electronicsHighpassHz, 0.5, 30.0);
        if (spec.hasLinearizer)
            range("tape.linearizerHd3Percent", spec.linearizerHd3Percent, 0.05, spec.hd3PercentAtCal);
        if (spec.speeds.empty())
            problems.push_back("no speeds defined");
        for (const auto& s : spec.speeds)
        {
            const std::string tag = "speeds[" + std::to_string(static_cast<int>(s.ips * 10) / 10.0).substr(0, 4) + "].";
            range(tag + "ips", s.ips, 1.0, 60.0);
            range(tag + "hfTimeConstantUs", s.hfTimeConstantUs, 5.0, 200.0);
            range(tag + "playbackHfCornerHz", s.playbackHfCornerHz, 8000.0, 60000.0);
            range(tag + "flutterLowBandRmsPercent", s.flutterLowBandRmsPercent, 0.0, 0.5);
            range(tag + "snrDb", s.snrDb, 40.0, 90.0);
        }

        static const char* required[] = {
            "format.trackWidthInches", "tape.coatingThicknessMicrons", "tape.hd3PercentAtCal", "tape.hd3CalLevelDb",
            "head.headBumpWavelengthInches", "head.headBumpGainDb", "electronics.transformers", "electronics.maxOutputDb",
            "electronics.highpassHz", "transport.flutterComponents", "speeds.equalization", "speeds.playbackHfCornerHz",
            "speeds.flutter", "speeds.snrDb", "referenceDbfs" };
        for (const char* key : required)
        {
            const auto it = spec.evidence.find(key);
            if (it == spec.evidence.end())
            {
                problems.push_back(std::string("no evidence entry for ") + key);
                continue;
            }
            const std::string& e = it->second;
            const bool ok = e.rfind("published:", 0) == 0 || e.rfind("standard:", 0) == 0 || e.rfind("derived:", 0) == 0 || e.rfind("assumed:", 0) == 0;
            if (!ok)
                problems.push_back(std::string("evidence for ") + key + " must start with published:, standard:, derived: or assumed:");
        }
        return problems;
    }

    bool parseTapeFormula(const std::string& jsonText, TapeFormula& out, std::string& error)
    {
        try
        {
            Parser parser(jsonText);
            const Value root = parser.parseDocument();
            if (root.kind != Value::Kind::Object)
                throw std::runtime_error("formula must be a JSON object");
            if (text(root, "schema", true) != "tapecore.formula.v1")
                throw std::runtime_error("schema must be 'tapecore.formula.v1'");
            TapeFormula f;
            f.id = text(root, "id", true);
            f.displayName = text(root, "displayName", true);
            f.referenceTape = text(root, "referenceTape");
            f.status = text(root, "status", true);
            f.coatingThicknessMicrons = number(root, "coatingThicknessMicrons", 0, true);
            f.molDb = number(root, "molDb", 0, true);
            f.hd3PercentAtOperatingLevel = number(root, "hd3PercentAtOperatingLevel", 0.0);
            f.overbiasDb = number(root, "overbiasDb", 0, true);
            f.overbiasReferenceHz = number(root, "overbiasReferenceHz", f.overbiasReferenceHz);
            if (const Value* evidence = root.find("evidence"); evidence != nullptr && evidence->kind == Value::Kind::Object)
                for (const auto& m : evidence->members)
                    if (m.second.kind == Value::Kind::String)
                        f.evidence[m.first] = m.second.text;
            out = std::move(f);
            error.clear();
            return true;
        }
        catch (const std::exception& e)
        {
            error = e.what();
            return false;
        }
    }

    bool loadTapeFormulaFile(const std::string& path, TapeFormula& out, std::string& error)
    {
        std::ifstream file(path);
        if (!file)
        {
            error = "cannot open formula '" + path + "'";
            return false;
        }
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return parseTapeFormula(buffer.str(), out, error);
    }

    std::vector<std::string> validateTapeFormula(const TapeFormula& f)
    {
        std::vector<std::string> problems;
        auto range = [&](const std::string& name, double v, double lo, double hi)
        {
            if (!(v >= lo && v <= hi))
                problems.push_back(name + " = " + std::to_string(v) + " is outside " + std::to_string(lo) + ".." + std::to_string(hi));
        };
        range("coatingThicknessMicrons", f.coatingThicknessMicrons, 3.0, 30.0);
        range("molDb", f.molDb, 0.0, 20.0);
        range("overbiasDb", f.overbiasDb, 0.0, 8.0);
        range("overbiasReferenceHz", f.overbiasReferenceHz, 1000.0, 20000.0);
        for (const char* key : { "coatingThicknessMicrons", "molDb", "overbias" })
        {
            const auto it = f.evidence.find(key);
            if (it == f.evidence.end())
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
