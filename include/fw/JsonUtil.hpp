#ifndef ALKAIDLAB_FW_JSON_UTIL_HPP
#define ALKAIDLAB_FW_JSON_UTIL_HPP

#include <nlohmann/json.hpp>
#include <string>

namespace alkaidlab {
namespace fw {

class JsonUtil {
public:
    using json = nlohmann::json;

    static std::string getString(const std::string& body, const std::string& key,
                                 const std::string& defaultValue = "") {
        try {
            json j = json::parse(body);
            if (j.contains(key)) {
                const auto& v = j[key];
                if (v.is_string()) {
                    return v.get<std::string>();
                }
                if (v.is_number_integer()) {
                    return std::to_string(v.get<int64_t>());
                }
                if (v.is_number_float()) {
                    return std::to_string(v.get<double>());
                }
            }
        } catch (...) {}
        return defaultValue;
    }

    static int getInt(const std::string& body, const std::string& key, int defaultValue = 0) {
        try {
            json j = json::parse(body);
            if (j.contains(key) && j[key].is_number_integer()) {
                return j[key].get<int>();
            }
        } catch (...) {}
        return defaultValue;
    }

    static bool getBool(const std::string& body, const std::string& key, bool defaultValue = false) {
        try {
            json j = json::parse(body);
            if (j.contains(key)) {
                const auto& v = j[key];
                if (v.is_boolean()) {
                    return v.get<bool>();
                }
                if (v.is_number_integer()) {
                    return v.get<int>() != 0;
                }
                if (v.is_string()) {
                    std::string s = v.get<std::string>();
                    return (s == "true" || s == "1");
                }
            }
        } catch (...) {}
        return defaultValue;
    }

    static json safeParse(const std::string& body) {
        try { return json::parse(body); } catch (...) { return json::object(); }
    }

    static std::string escape(const std::string& s) {
        std::string out;
        out.reserve(s.size() + 8);
        for (char c : s) {
            if (c == '"') {
                out += "\\\"";
            } else if (c == '\\') {
                out += "\\\\";
            } else if (c == '\n') {
                out += "\\n";
            } else if (c == '\r') {
                out += "\\r";
            } else if (c == '\t') {
                out += "\\t";
            } else if (static_cast<unsigned char>(c) < 32) {
                continue;
            } else {
                out += c;
            }
        }
        return out;
    }
};

} // namespace fw
} // namespace alkaidlab

#endif // ALKAIDLAB_FW_JSON_UTIL_HPP
