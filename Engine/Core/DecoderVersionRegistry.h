// DecoderVersionRegistry.h — Decoder Version Registry with SemVer Compatibility
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks registered decoders with semantic versioning, enforces API compatibility,
// and provides upgrade path resolution for decoder migrations.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

struct SemVer {
    uint16_t major = 0;
    uint16_t minor = 0;
    uint16_t patch = 0;
    std::string prerelease;

    bool operator==(const SemVer& o) const {
        return major == o.major && minor == o.minor && patch == o.patch;
    }
    bool operator<(const SemVer& o) const {
        if (major != o.major) return major < o.major;
        if (minor != o.minor) return minor < o.minor;
        return patch < o.patch;
    }
    bool IsCompatibleWith(const SemVer& required) const {
        return major == required.major && (minor > required.minor ||
               (minor == required.minor && patch >= required.patch));
    }
    std::string ToString() const {
        std::string s = std::to_string(major) + "." + std::to_string(minor) +
                        "." + std::to_string(patch);
        if (!prerelease.empty()) s += "-" + prerelease;
        return s;
    }
    static SemVer Parse(const std::string& str) {
        SemVer v;
        size_t p1 = str.find('.'), p2 = str.find('.', p1 + 1);
        if (p1 == std::string::npos) return v;
        v.major = static_cast<uint16_t>(std::stoi(str.substr(0, p1)));
        v.minor = static_cast<uint16_t>(std::stoi(str.substr(p1 + 1, p2 - p1 - 1)));
        auto dashPos = str.find('-', p2 + 1);
        if (dashPos != std::string::npos) {
            v.patch = static_cast<uint16_t>(std::stoi(str.substr(p2 + 1, dashPos - p2 - 1)));
            v.prerelease = str.substr(dashPos + 1);
        } else {
            v.patch = static_cast<uint16_t>(std::stoi(str.substr(p2 + 1)));
        }
        return v;
    }
};

struct DecoderRegistration {
    std::string name;
    SemVer version;
    SemVer apiVersion;
    std::string author;
    std::string description;
    bool isCompatible = true;
    uint64_t registeredAtMs = 0;
};

class DecoderVersionRegistry {
public:
    DecoderVersionRegistry() : m_apiVersion{1, 0, 0, ""} {}
    ~DecoderVersionRegistry() = default;

    bool RegisterDecoder(const DecoderRegistration& reg) {
        for (const auto& existing : m_decoders)
            if (existing.name == reg.name) return false;
        DecoderRegistration entry = reg;
        entry.isCompatible = entry.apiVersion.IsCompatibleWith(m_apiVersion);
        m_decoders.push_back(entry);
        return true;
    }

    const DecoderRegistration* GetDecoder(const std::string& name) const {
        for (const auto& d : m_decoders)
            if (d.name == name) return &d;
        return nullptr;
    }

    bool CheckCompatibility(const std::string& name) const {
        auto* d = GetDecoder(name);
        return d && d->isCompatible;
    }

    std::vector<DecoderRegistration> ListAll() const { return m_decoders; }

    std::vector<DecoderRegistration> GetIncompatible() const {
        std::vector<DecoderRegistration> result;
        for (const auto& d : m_decoders)
            if (!d.isCompatible) result.push_back(d);
        return result;
    }

    std::vector<std::string> GetUpgradePath(const std::string& decoderName,
                                             const SemVer& targetVersion) const {
        std::vector<std::string> path;
        auto* d = GetDecoder(decoderName);
        if (!d) return path;
        if (d->version < targetVersion) {
            path.push_back(d->version.ToString() + " -> " + targetVersion.ToString());
            if (d->version.major != targetVersion.major)
                path.push_back("WARNING: major version bump — breaking changes expected");
        }
        return path;
    }

    void SetApiVersion(const SemVer& version) { m_apiVersion = version; }
    const SemVer& GetApiVersion() const { return m_apiVersion; }
    size_t GetRegisteredCount() const { return m_decoders.size(); }

private:
    std::vector<DecoderRegistration> m_decoders;
    SemVer m_apiVersion;
};

}} // namespace ExplorerLens::Engine
