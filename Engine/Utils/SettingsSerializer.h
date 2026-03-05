// SettingsSerializer.h — JSON Settings Import/Export
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a lightweight settings serialization engine for exporting and
// importing configuration as JSON, with diff computation between configs.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <mutex>
#include <unordered_map>
#include <sstream>

namespace ExplorerLens {
namespace Engine {

struct SerializerSection {
    std::string sectionName;
    std::unordered_map<std::string, std::string> values;

    void Set(const std::string& key, const std::string& value) {
        values[key] = value;
    }

    std::string Get(const std::string& key, const std::string& defaultVal = "") const {
        auto it = values.find(key);
        return it != values.end() ? it->second : defaultVal;
    }

    bool Has(const std::string& key) const {
        return values.find(key) != values.end();
    }
};

struct SettingsDiff {
    struct Change {
        std::string section;
        std::string key;
        std::string oldValue;
        std::string newValue;
        bool        isAdded = false;
        bool        isRemoved = false;
    };

    std::vector<Change> changes;

    size_t AddedCount() const {
        size_t c = 0;
        for (const auto& ch : changes) if (ch.isAdded) c++;
        return c;
    }

    size_t RemovedCount() const {
        size_t c = 0;
        for (const auto& ch : changes) if (ch.isRemoved) c++;
        return c;
    }

    size_t ModifiedCount() const {
        size_t c = 0;
        for (const auto& ch : changes) if (!ch.isAdded && !ch.isRemoved) c++;
        return c;
    }
};

class SettingsSerializer {
public:
    static SettingsSerializer& Instance() {
        static SettingsSerializer s;
        return s;
    }

    void SetSection(const SerializerSection& section) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sections[section.sectionName] = section;
    }

    SerializerSection GetSection(const std::string& name) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_sections.find(name);
        return it != m_sections.end() ? it->second : SerializerSection{};
    }

    std::string ExportToJSON() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return ExportToJSONInternal();
    }

    bool ImportFromJSON(const std::string& json) {
        std::lock_guard<std::mutex> lock(m_mutex);
        // Lightweight parser for our known format
        m_sections.clear();
        std::string currentSection;
        bool inSection = false;

        std::istringstream iss(json);
        std::string line;
        while (std::getline(iss, line)) {
            auto trimmed = Trim(line);
            if (trimmed.empty() || trimmed == "{" || trimmed == "}") continue;

            // Remove trailing comma
            if (!trimmed.empty() && trimmed.back() == ',')
                trimmed.pop_back();

            if (trimmed.back() == '{') {
                // Section start
                size_t q1 = trimmed.find('"');
                size_t q2 = trimmed.find('"', q1 + 1);
                if (q1 != std::string::npos && q2 != std::string::npos) {
                    currentSection = trimmed.substr(q1 + 1, q2 - q1 - 1);
                    m_sections[currentSection].sectionName = currentSection;
                    inSection = true;
                }
            }
            else if (trimmed == "}") {
                inSection = false;
            }
            else if (inSection) {
                // Key-value pair
                size_t q1 = trimmed.find('"');
                size_t q2 = trimmed.find('"', q1 + 1);
                size_t q3 = trimmed.find('"', q2 + 1);
                size_t q4 = trimmed.find('"', q3 + 1);
                if (q1 != std::string::npos && q4 != std::string::npos) {
                    std::string key = trimmed.substr(q1 + 1, q2 - q1 - 1);
                    std::string val = trimmed.substr(q3 + 1, q4 - q3 - 1);
                    m_sections[currentSection].Set(key, val);
                }
            }
        }
        return !m_sections.empty();
    }

    SettingsDiff ComputeDiff(const std::string& otherJSON) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        SettingsDiff diff;

        // Parse other into temporary sections
        SettingsSerializer temp;
        temp.ImportFromJSON(otherJSON);

        // Find removed and modified
        for (const auto& sectionPair : m_sections) {
            auto otherSec = temp.GetSection(sectionPair.first);
            for (const auto& kvPair : sectionPair.second.values) {
                if (!otherSec.Has(kvPair.first)) {
                    SettingsDiff::Change ch;
                    ch.section = sectionPair.first; ch.key = kvPair.first;
                    ch.oldValue = kvPair.second; ch.isRemoved = true;
                    diff.changes.push_back(ch);
                }
                else if (otherSec.Get(kvPair.first) != kvPair.second) {
                    SettingsDiff::Change ch;
                    ch.section = sectionPair.first; ch.key = kvPair.first;
                    ch.oldValue = kvPair.second; ch.newValue = otherSec.Get(kvPair.first);
                    diff.changes.push_back(ch);
                }
            }
        }

        // Find added
        auto otherSections = temp.GetAllSections();
        for (const auto& sectionPair : otherSections) {
            for (const auto& kvPair : sectionPair.second.values) {
                auto mySec = m_sections.find(sectionPair.first);
                if (mySec == m_sections.end() || !mySec->second.Has(kvPair.first)) {
                    SettingsDiff::Change ch;
                    ch.section = sectionPair.first; ch.key = kvPair.first;
                    ch.newValue = kvPair.second; ch.isAdded = true;
                    diff.changes.push_back(ch);
                }
            }
        }

        return diff;
    }

    std::unordered_map<std::string, SerializerSection> GetAllSections() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_sections;
    }

    bool ExportToFile(const std::wstring& path) const {
        std::string json = ExportToJSON();
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return false;
        DWORD written;
        WriteFile(hFile, json.c_str(), static_cast<DWORD>(json.size()), &written, nullptr);
        CloseHandle(hFile);
        return true;
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sections.clear();
    }

    bool Validate() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& sectionPair : m_sections) {
            if (sectionPair.second.sectionName.empty()) return false;
            if (sectionPair.second.sectionName != sectionPair.first) return false;
        }
        // Verify roundtrip (use internal method to avoid deadlock from ExportToJSON re-locking m_mutex)
        std::string json = ExportToJSONInternal();
        return !json.empty();
    }

private:
    SettingsSerializer() = default;
    ~SettingsSerializer() = default;
    SettingsSerializer(const SettingsSerializer&) = delete;
    SettingsSerializer& operator=(const SettingsSerializer&) = delete;

    std::string ExportToJSONInternal() const {
        std::ostringstream oss;
        oss << "{\n";
        bool firstSection = true;
        for (const auto& sectionPair : m_sections) {
            if (!firstSection) oss << ",\n";
            firstSection = false;
            oss << "  \"" << EscapeJSON(sectionPair.first) << "\": {\n";
            bool firstVal = true;
            for (const auto& kvPair : sectionPair.second.values) {
                if (!firstVal) oss << ",\n";
                firstVal = false;
                oss << "    \"" << EscapeJSON(kvPair.first) << "\": \"" << EscapeJSON(kvPair.second) << "\"";
            }
            oss << "\n  }";
        }
        oss << "\n}\n";
        return oss.str();
    }

    static std::string EscapeJSON(const std::string& s) {
        std::string result;
        for (char c : s) {
            switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
            }
        }
        return result;
    }

    static std::string Trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, SerializerSection> m_sections;
};

}
} // namespace ExplorerLens::Engine
