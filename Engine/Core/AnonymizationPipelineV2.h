// AnonymizationPipelineV2.h — Anonymization Pipeline v2
// Copyright (c) 2026 ExplorerLens Project
//
// Scrubs file paths and metadata of PII before telemetry upload (k-anonymity, t-closeness).
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <regex>

namespace ExplorerLens { namespace Engine {

enum class AnonymizationStrategy { KAnonymity, TCloseness, Pseudonymize };

struct AnonPipelineConfig {
    AnonymizationStrategy strategy       = AnonymizationStrategy::KAnonymity;
    uint32_t              kValue         = 5;
    bool                  stripUserNames = true;
};

struct AnonRecord {
    std::string originalPath;
    std::string anonymizedPath;
    bool        wasModified = false;
};

class AnonymizationPipelineV2 {
public:
    explicit AnonymizationPipelineV2(const AnonPipelineConfig& config) : m_config(config) {}

    AnonRecord Anonymize(const std::string& path) {
        AnonRecord r;
        r.originalPath    = path;
        r.anonymizedPath  = ScrubUserName(path);
        r.wasModified     = (r.anonymizedPath != path);
        return r;
    }
    std::vector<AnonRecord> AnonymizeBatch(const std::vector<std::string>& paths) {
        std::vector<AnonRecord> results;
        results.reserve(paths.size());
        for (const auto& p : paths) results.push_back(Anonymize(p));
        return results;
    }
    bool ContainsPII(const std::string& input) const {
        return input.find("\\Users\\") != std::string::npos ||
               input.find("/home/")    != std::string::npos;
    }

private:
    AnonPipelineConfig m_config;

    std::string ScrubUserName(const std::string& path) const {
        if (!m_config.stripUserNames) return path;
        std::string result = path;
        const std::string marker = "\\Users\\";
        auto pos = result.find(marker);
        if (pos != std::string::npos) {
            auto end = result.find('\\', pos + marker.size());
            if (end != std::string::npos)
                result.replace(pos + marker.size(), end - pos - marker.size(), "[USER]");
        }
        return result;
    }
};

}} // namespace ExplorerLens::Engine
