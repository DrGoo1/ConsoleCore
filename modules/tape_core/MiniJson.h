#pragma once
// Minimal JSON reader shared by the profile loaders. Standard library only.
// Not for use on the audio thread. Functions throw std::runtime_error on bad input.

#include <cctype>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace tape::json
{
    // Minimal JSON value tree: enough for machine profiles.
    struct Value
    {
        enum class Kind { Null, Bool, Number, String, Array, Object } kind = Kind::Null;
        bool boolean = false;
        double number = 0.0;
        std::string text;
        std::vector<Value> items;
        std::vector<std::pair<std::string, Value>> members;

        const Value* find(const std::string& key) const
        {
            for (const auto& m : members)
                if (m.first == key)
                    return &m.second;
            return nullptr;
        }
    };

    class Parser
    {
    public:
        explicit Parser(const std::string& s) : src(s) {}

        Value parseDocument()
        {
            Value v = parseValue(0);
            skipSpace();
            if (pos != src.size())
                fail("unexpected text after the JSON document");
            return v;
        }

    private:
        [[noreturn]] void fail(const std::string& what) const
        {
            throw std::runtime_error(what + " at character " + std::to_string(pos));
        }

        void skipSpace()
        {
            while (pos < src.size() && std::isspace(static_cast<unsigned char>(src[pos])))
                ++pos;
        }

        Value parseValue(int depth)
        {
            if (depth > 16)
                fail("JSON nested too deeply");
            skipSpace();
            if (pos >= src.size())
                fail("unexpected end of JSON");
            const char c = src[pos];
            Value v;
            if (c == '{')
            {
                v.kind = Value::Kind::Object;
                ++pos;
                skipSpace();
                if (pos < src.size() && src[pos] == '}') { ++pos; return v; }
                for (;;)
                {
                    skipSpace();
                    std::string key = parseString();
                    skipSpace();
                    if (pos >= src.size() || src[pos] != ':')
                        fail("expected ':' after object key");
                    ++pos;
                    v.members.emplace_back(std::move(key), parseValue(depth + 1));
                    skipSpace();
                    if (pos < src.size() && src[pos] == ',') { ++pos; continue; }
                    if (pos < src.size() && src[pos] == '}') { ++pos; return v; }
                    fail("expected ',' or '}' in object");
                }
            }
            if (c == '[')
            {
                v.kind = Value::Kind::Array;
                ++pos;
                skipSpace();
                if (pos < src.size() && src[pos] == ']') { ++pos; return v; }
                for (;;)
                {
                    v.items.push_back(parseValue(depth + 1));
                    skipSpace();
                    if (pos < src.size() && src[pos] == ',') { ++pos; continue; }
                    if (pos < src.size() && src[pos] == ']') { ++pos; return v; }
                    fail("expected ',' or ']' in array");
                }
            }
            if (c == '"')
            {
                v.kind = Value::Kind::String;
                v.text = parseString();
                return v;
            }
            if (src.compare(pos, 4, "true") == 0) { pos += 4; v.kind = Value::Kind::Bool; v.boolean = true; return v; }
            if (src.compare(pos, 5, "false") == 0) { pos += 5; v.kind = Value::Kind::Bool; return v; }
            if (src.compare(pos, 4, "null") == 0) { pos += 4; return v; }
            char* end = nullptr;
            v.number = std::strtod(src.c_str() + pos, &end);
            if (end == src.c_str() + pos)
                fail("unexpected character in JSON");
            pos = static_cast<std::size_t>(end - src.c_str());
            v.kind = Value::Kind::Number;
            return v;
        }

        std::string parseString()
        {
            if (pos >= src.size() || src[pos] != '"')
                fail("expected a string");
            ++pos;
            std::string out;
            while (pos < src.size() && src[pos] != '"')
            {
                char c = src[pos++];
                if (c == '\\')
                {
                    if (pos >= src.size())
                        fail("unterminated escape");
                    const char e = src[pos++];
                    switch (e)
                    {
                        case 'n': c = '\n'; break;
                        case 't': c = '\t'; break;
                        case '"': case '\\': case '/': c = e; break;
                        default: fail("unsupported escape in string");
                    }
                }
                out.push_back(c);
            }
            if (pos >= src.size())
                fail("unterminated string");
            ++pos;
            return out;
        }

        const std::string& src;
        std::size_t pos = 0;
    };

    inline double number(const Value& obj, const std::string& key, double fallback, bool required = false)
    {
        const Value* v = obj.find(key);
        if (v == nullptr || v->kind != Value::Kind::Number)
        {
            if (required)
                throw std::runtime_error("missing number '" + key + "'");
            return fallback;
        }
        return v->number;
    }

    inline std::string text(const Value& obj, const std::string& key, bool required = false)
    {
        const Value* v = obj.find(key);
        if (v == nullptr || v->kind != Value::Kind::String)
        {
            if (required)
                throw std::runtime_error("missing string '" + key + "'");
            return {};
        }
        return v->text;
    }

    inline bool flag(const Value& obj, const std::string& key, bool fallback)
    {
        const Value* v = obj.find(key);
        return (v != nullptr && v->kind == Value::Kind::Bool) ? v->boolean : fallback;
    }

    inline const Value* object(const Value& obj, const char* key)
    {
        const Value* v = obj.find(key);
        if (v == nullptr || v->kind != Value::Kind::Object)
            throw std::runtime_error(std::string("missing object '") + key + "'");
        return v;
    }
}
