// AdaptiveColorProfileManager.h — Adaptive Color Profile Management
// Copyright (c) 2026 ExplorerLens Project
//
// Manages color profile selection and adaptation based on display capabilities,
// content type, and user preferences. Provides automatic ICC profile matching
// and HDR tone-mapping decisions. Singleton with Initialize/Shutdown lifecycle.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class AdaptiveColorSpace : uint8_t {
    SRGB,
    AdobeRGB,
    DisplayP3,
    ProPhotoRGB,
    Rec2020,
    LinearRGB,
    HDR10
};

enum class ToneMappingMode : uint8_t {
    None,
    Reinhard,
    ACES,
    Filmic,
    AgX,
    Auto
};

struct AdaptiveColorProfile {
    AdaptiveColorSpace sourceSpace = AdaptiveColorSpace::SRGB;
    AdaptiveColorSpace targetSpace = AdaptiveColorSpace::SRGB;
    ToneMappingMode toneMapping = ToneMappingMode::None;
    float gamma = 2.2f;
    bool hdrCapable = false;
    float maxLuminanceNits = 100.0f;
};

struct ColorProfileStats {
    uint64_t totalProfileMatches = 0;
    uint64_t hdrConversions = 0;
    uint64_t gamutMappings = 0;
    uint64_t fallbacksToSRGB = 0;
    bool initialized = false;
};

class AdaptiveColorProfileManager {
public:
    static AdaptiveColorProfileManager& Instance() {
        static AdaptiveColorProfileManager instance;
        return instance;
    }

    void Initialize(AdaptiveColorSpace displaySpace = AdaptiveColorSpace::SRGB,
                    float displayMaxNits = 100.0f) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_displaySpace = displaySpace;
        m_displayMaxNits = displayMaxNits;
        m_stats = {};
        m_stats.initialized = true;
    }

    AdaptiveColorProfile MatchProfile(AdaptiveColorSpace sourceSpace) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.totalProfileMatches++;

        AdaptiveColorProfile profile;
        profile.sourceSpace = sourceSpace;
        profile.targetSpace = m_displaySpace;
        profile.maxLuminanceNits = m_displayMaxNits;
        profile.hdrCapable = (m_displayMaxNits > 400.0f);

        if (sourceSpace == m_displaySpace) {
            profile.toneMapping = ToneMappingMode::None;
        } else if (sourceSpace == AdaptiveColorSpace::HDR10 ||
                   sourceSpace == AdaptiveColorSpace::Rec2020) {
            if (profile.hdrCapable) {
                profile.toneMapping = ToneMappingMode::ACES;
                m_stats.hdrConversions++;
            } else {
                profile.toneMapping = ToneMappingMode::Reinhard;
                profile.targetSpace = AdaptiveColorSpace::SRGB;
                m_stats.fallbacksToSRGB++;
            }
        } else {
            profile.toneMapping = ToneMappingMode::None;
            m_stats.gamutMappings++;
        }

        return profile;
    }

    bool IsInitialized() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats.initialized;
    }

    ColorProfileStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    void Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.initialized = false;
    }

private:
    AdaptiveColorProfileManager() = default;
    ~AdaptiveColorProfileManager() = default;
    AdaptiveColorProfileManager(const AdaptiveColorProfileManager&) = delete;
    AdaptiveColorProfileManager& operator=(const AdaptiveColorProfileManager&) = delete;

    mutable std::mutex m_mutex;
    AdaptiveColorSpace m_displaySpace = AdaptiveColorSpace::SRGB;
    float m_displayMaxNits = 100.0f;
    ColorProfileStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
