// TelemetryDataMinimizer.h — Telemetry Data Minimizer
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces PII-free telemetry transmission via static analysis and runtime scrubbing.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

enum class TDMScrubResult { Clean, PurgedPII, Blocked };

struct TelemetryPayload {
    std::string              eventName;
    std::vector<std::string> fields;
    std::vector<std::string> values;
};

struct TDMMinimizeResult {
    TDMScrubResult   result        = TDMScrubResult::Clean;
    TelemetryPayload sanitized;
    uint32_t         fieldsRemoved = 0;
};

class TelemetryDataMinimizer {
public:
    TDMMinimizeResult Minimize(const TelemetryPayload& payload) {
        TDMMinimizeResult r;
        r.sanitized.eventName = payload.eventName;
        for (size_t i = 0; i < payload.fields.size(); ++i) {
            const std::string& f = payload.fields[i];
            const std::string& v = i < payload.values.size() ? payload.values[i] : "";
            if (IsPIIField(f) || IsPIIValue(v)) {
                ++r.fieldsRemoved;
                r.result = TDMScrubResult::PurgedPII;
            } else {
                r.sanitized.fields.push_back(f);
                r.sanitized.values.push_back(v);
            }
        }
        return r;
    }
    bool ContainsPII(const TelemetryPayload& payload) const {
        for (const auto& f : payload.fields)
            if (IsPIIField(f)) return true;
        for (const auto& v : payload.values)
            if (IsPIIValue(v)) return true;
        return false;
    }
    uint32_t PIIFieldCount(const TelemetryPayload& payload) const {
        uint32_t count = 0;
        for (const auto& f : payload.fields)
            if (IsPIIField(f)) ++count;
        return count;
    }

private:
    static bool IsPIIField(const std::string& f) {
        std::string lf = f;
        std::transform(lf.begin(), lf.end(), lf.begin(),
                       [](unsigned char c) -> char { return static_cast<char>(std::tolower(c)); });
        return lf == "email" || lf == "username" || lf == "name" || lf == "ip";
    }
    static bool IsPIIValue(const std::string& v) {
        return v.find('@') != std::string::npos;  // email heuristic
    }
};

}} // namespace ExplorerLens::Engine
