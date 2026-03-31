// DecoderVersionManager.h — Decoder Version Tracking and Compatibility
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks decoder versions, checks compatibility, and manages version-specific
// feature flags. Supports automatic decoder upgrade detection and migration
// paths. Singleton with Initialize/Shutdown lifecycle.
//
#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct DecoderVersionInfo {
    std::wstring decoderName;
    uint32_t majorVersion = 0;
    uint32_t minorVersion = 0;
    uint32_t patchVersion = 0;
    bool supportsGPU = false;
    bool supportsStreaming = false;
    bool deprecated = false;
};

enum class VersionCompatibility : uint8_t {
    FullyCompatible,
    BackwardCompatible,
    MinorIncompatibility,
    MajorIncompatibility,
    Unsupported
};

struct VersionCheckResult {
    std::wstring decoderName;
    VersionCompatibility compatibility = VersionCompatibility::FullyCompatible;
    bool upgradeAvailable = false;
    std::wstring upgradeMessage;
};

struct DecoderVersionStats {
    uint64_t totalChecks = 0;
    uint32_t registeredDecoders = 0;
    uint32_t deprecatedDecoders = 0;
    uint32_t gpuCapableDecoders = 0;
    bool initialized = false;
};

class DecoderVersionManager {
public:
    static DecoderVersionManager& Instance() {
        static DecoderVersionManager instance;
        return instance;
    }

    void Initialize() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_decoders.clear();
        m_stats = {};
        m_stats.initialized = true;
    }

    void RegisterDecoder(const DecoderVersionInfo& info) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_decoders[info.decoderName] = info;
        m_stats.registeredDecoders = static_cast<uint32_t>(m_decoders.size());
        if (info.deprecated) m_stats.deprecatedDecoders++;
        if (info.supportsGPU) m_stats.gpuCapableDecoders++;
    }

    VersionCheckResult CheckCompatibility(const std::wstring& decoderName,
                                           uint32_t requiredMajor,
                                           uint32_t requiredMinor = 0) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.totalChecks++;

        VersionCheckResult result;
        result.decoderName = decoderName;

        auto it = m_decoders.find(decoderName);
        if (it == m_decoders.end()) {
            result.compatibility = VersionCompatibility::Unsupported;
            return result;
        }

        const auto& info = it->second;
        if (info.deprecated) {
            result.compatibility = VersionCompatibility::MinorIncompatibility;
            result.upgradeAvailable = true;
            result.upgradeMessage = L"Decoder is deprecated";
        } else if (info.majorVersion > requiredMajor) {
            result.compatibility = VersionCompatibility::FullyCompatible;
        } else if (info.majorVersion == requiredMajor && info.minorVersion >= requiredMinor) {
            result.compatibility = VersionCompatibility::FullyCompatible;
        } else if (info.majorVersion == requiredMajor) {
            result.compatibility = VersionCompatibility::BackwardCompatible;
            result.upgradeAvailable = true;
        } else {
            result.compatibility = VersionCompatibility::MajorIncompatibility;
            result.upgradeAvailable = true;
        }

        return result;
    }

    DecoderVersionInfo GetDecoderInfo(const std::wstring& decoderName) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_decoders.find(decoderName);
        if (it != m_decoders.end()) return it->second;
        return {};
    }

    bool IsInitialized() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats.initialized;
    }

    DecoderVersionStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    void Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.initialized = false;
        m_decoders.clear();
    }

private:
    DecoderVersionManager() = default;
    ~DecoderVersionManager() = default;
    DecoderVersionManager(const DecoderVersionManager&) = delete;
    DecoderVersionManager& operator=(const DecoderVersionManager&) = delete;

    mutable std::mutex m_mutex;
    std::unordered_map<std::wstring, DecoderVersionInfo> m_decoders;
    DecoderVersionStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
